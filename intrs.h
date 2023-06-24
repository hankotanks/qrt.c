#ifndef INTRS_H
#define INTRS_H

#include<assert.h>
#include<float.h>

#include "geom.h"

#define MIN(i, j) (((i) < (j)) ? (i) : (j))
#define MAX(i, j) (((i) > (j)) ? (i) : (j))

#define EPS_BVH 0.2

//
// `Surface` declaration

typedef enum SurfaceType { NONE = 0, TRI, SPHERE } SurfaceType;

typedef struct Surface {
    SurfaceType st;
    union {
        Tri* tri;
        Sphere* sphere;
    };
} Surface;

//
// `Surface` functions

int surface_match(Surface a, Surface b) {
    switch(a.st) {
        case SPHERE:
            return b.st == SPHERE && a.sphere == b.sphere;
        case TRI:
            return b.st == TRI && a.tri == b.tri;
        case NONE:
            return b.st == NONE;
    }

    return 0;
}

void surface_print_internal(Surface* s, char* name, size_t indent) {
    int id = 4 * (int) indent;

    if(name)
        printf("%.*s%s (surface) {\n", id, SP, name);
    else 
        printf("%.*s surface {\n", id, SP);

    switch(s->st) {
        case TRI:
            tri_print_internal(s->tri, NULL, indent + 1);
            break;
        case SPHERE:
            sphere_print_internal(s->sphere, NULL, indent + 1);
            break;
        case NONE: 
            printf("%.*s`NONE`\n", id + 4, SP);
    } printf("%.*s}\n", id, SP);
}

void surface_print(Surface* s) {
    surface_print_internal(s, NULL, 0);
}

//
// `BVH` declaration

typedef struct BVH BVH;

struct BVH {
    BVH* l;
    BVH* r;
    Vec minima;
    Vec maxima;
    size_t c;
    Surface* surfaces;
};


//
// `BVH` functions

void helper_bvh_push_extrema(Vec a, Vec* minima, Vec* maxima) {
    if(a.x < minima->x) minima->x = a.x;
    if(a.y < minima->y) minima->y = a.y;
    if(a.z < minima->z) minima->z = a.z;

    if(a.x > maxima->x) maxima->x = a.x;
    if(a.y > maxima->y) maxima->y = a.y;
    if(a.z > maxima->z) maxima->z = a.z;
}

void helper_bvh_extrema(size_t c, const Surface* surfaces, Vec* minima, Vec* maxima) {
    *minima = vec_aaa(DBL_MAX);
    *maxima = vec_aaa(-1. * DBL_MAX);

    size_t i;
    for(i = 0; i < c; i++) {
        Surface s = surfaces[i];
        switch(s.st) {
            case TRI: {
                helper_bvh_push_extrema(s.tri->a.point, minima, maxima);
                helper_bvh_push_extrema(s.tri->b.point, minima, maxima);
                helper_bvh_push_extrema(s.tri->c.point, minima, maxima);
            }; break;
            case SPHERE: {
                Vec a = vec_aaa(s.sphere->radius);

                Vec mn = sub_vv(s.sphere->center, a);
                Vec mx = add_vv(s.sphere->center, a);

                helper_bvh_push_extrema(mn, minima, maxima);
                helper_bvh_push_extrema(mx, minima, maxima);
            }; break;
            case NONE: continue;
        }
    }
}

// Forward definition of the hierarchy split function
void bvh_split(BVH* h);

BVH* bvh_initialize(Mesh* meshes, Sphere* spheres) {
    assert(meshes || spheres);

    Mesh* curr_mesh = meshes;

    size_t tc = 0, sc = 0, c = 0;
    while(curr_mesh) {
        tc += curr_mesh->tc;
        curr_mesh = curr_mesh->next;
    }

    Sphere* curr_sphere = spheres;
    while(curr_sphere) {
        curr_sphere = curr_sphere->next;
        ++sc;
    } assert(tc || sc);

    Surface* surfaces = malloc((tc + sc) * (sizeof *surfaces));

    curr_mesh = meshes;
    while(curr_mesh) {
        size_t n;
        for(n = 0; n < curr_mesh->tc; n++) {
            surfaces[c] = (Surface) {
                .st = TRI,
                .tri = &curr_mesh->tris[n]
            };

            ++c;
        }

        curr_mesh = curr_mesh->next;
    }

    curr_sphere = spheres;
    while(curr_sphere) {
        surfaces[c] = (Surface) {
            .st = SPHERE,
            .sphere = curr_sphere
        };

        curr_sphere = curr_sphere->next;

        ++c;
    }

    Vec minima, maxima;
    helper_bvh_extrema(tc + sc, surfaces, &minima, &maxima);

    BVH* h = malloc(sizeof *h);
    *h = (BVH) {
        .l = NULL,
        .r = NULL,
        .minima = minima,
        .maxima = maxima,
        .c = tc + sc,
        .surfaces = surfaces
    };
    
    bvh_split(h);
    
    return h;
}

void bvh_free(BVH* h) {
    if(h->l) bvh_free(h->l);
    if(h->r) bvh_free(h->r);
    
    free(h->surfaces);
    free(h);
}

//
// Helper functions

int helper_bvh_contains_point(Vec minima, Vec maxima, Vec p) {
    return (
           p.x >= minima.x
        && p.x <= maxima.x
        && p.y >= minima.y
        && p.y <= maxima.y
        && p.z >= minima.z
        && p.z <= maxima.z
    );
}

int helper_bvh_contains(Vec minima, Vec maxima, Surface s) {
    switch(s.st) {
        case TRI:
            return helper_bvh_contains_point(minima, maxima, s.tri->centroid);
        case SPHERE:
            return helper_bvh_contains_point(minima, maxima, s.sphere->center);
        case NONE: return 0;
    }

    return 0;
}

int helper_bvh_scale_check(double d, BVH* l, BVH* r) {
    if(d < 0.5 * EPS_BVH) { // 0.000002
        free(l->surfaces);
        free(l);
        free(r->surfaces);
        free(r);

        return 1;
    }

    return 0;
}

int helper_bvh_ray_collides(BVH* h, Ray r) {
    r.dir = inv_v(r.dir);

    double t_min = 0.0;
    double t_max = DBL_MAX;
    
    double t0, t1;
    t0 = (h->minima.x - EPS_BVH - r.origin.x) * r.dir.x;
    t1 = (h->maxima.x + EPS_BVH - r.origin.x) * r.dir.x;

    t_min = MAX(t_min, MIN(t0, t1));
    t_max = MIN(t_max, MAX(t0, t1)); 

    t0 = (h->minima.y - EPS_BVH - r.origin.y) * r.dir.y;
    t1 = (h->maxima.y + EPS_BVH - r.origin.y) * r.dir.y;

    t_min = MAX(t_min, MIN(t0, t1));
    t_max = MIN(t_max, MAX(t0, t1)); 

    t0 = (h->minima.z - EPS_BVH - r.origin.z) * r.dir.z;
    t1 = (h->maxima.z + EPS_BVH - r.origin.z) * r.dir.z;

    t_min = MAX(t_min, MIN(t0, t1));
    t_max = MIN(t_max, MAX(t0, t1)); 

    return t_min < t_max;
}

//
// `BVH` split functionality

void bvh_split(BVH* h) {
    if(h->c == 1) return;

    double dx, dy, dz;
    dx = h->maxima.x - h->minima.x;
    dy = h->maxima.y - h->minima.y;
    dz = h->maxima.z - h->minima.z;

    BVH* l = malloc(sizeof *l);
    *l = (BVH) {
        .l = NULL,
        .r = NULL,
        .minima = h->minima,
        .maxima = h->maxima,
        .c = 0,
        .surfaces = malloc(h->c * (sizeof *l->surfaces))
    };

    BVH* r = malloc(sizeof *r);
    *r = (BVH) {
        .l = NULL,
        .r = NULL,
        .minima = h->minima,
        .maxima = h->maxima,
        .c = 0,
        .surfaces = malloc(h->c * (sizeof *r->surfaces))
    };

    if(dx >= dy && dx >= dz) { // dx
        if(helper_bvh_scale_check(dx, l, r)) return;

        l->maxima.x = h->minima.x + 0.5 * dx;
        r->minima.x = l->maxima.x;
    } else if(dy >= dz && dy >= dx) { // dy
        if(helper_bvh_scale_check(dz, l, r)) return;

        l->maxima.y = h->minima.y + 0.5 * dy;
        r->minima.y = l->maxima.y;
    } else { // dz
        if(helper_bvh_scale_check(dz, l, r)) return;

        l->maxima.z = h->minima.z + 0.5 * dz;
        r->minima.z = l->maxima.z;
    }

    size_t i;
    for(i = 0; i < h->c; i++) {
        if(helper_bvh_contains(l->minima, l->maxima, h->surfaces[i])) {
            l->surfaces[l->c] = h->surfaces[i];
            ++(l->c);
        } else {
            r->surfaces[r->c] = h->surfaces[i];
            ++(r->c);
        }
    }

    if(l->c == 0) {
        h->minima = r->minima;
        h->maxima = r->maxima;

        helper_bvh_scale_check(0., l, r);

        bvh_split(h);
    } else if(r->c == 0) {
        h->minima = l->minima;
        h->maxima = l->maxima;

        helper_bvh_scale_check(0., l, r);

        bvh_split(h);
    } else {
        l->surfaces = realloc(l->surfaces, l->c * (sizeof *l->surfaces));
        assert(l->surfaces);

        r->surfaces = realloc(r->surfaces, r->c * (sizeof *r->surfaces));
        assert(r->surfaces);

        helper_bvh_extrema(l->c, l->surfaces, &l->minima, &l->maxima);
        bvh_split(l);

        helper_bvh_extrema(r->c, r->surfaces, &r->minima, &r->maxima);
        bvh_split(r);

        h->l = l;
        h->r = r;
    }
}

void bvh_print_internal(BVH* h, size_t indent) {
    int id = 4 * (int) indent;

    printf("%.*sbvh (%u) {\n", id, SP, (unsigned) h->c);

    if(!h->l && !h->r) {
        size_t i;
        for(i = 0; i < h->c; i++)
            surface_print_internal(&h->surfaces[i], NULL, indent + 1);
    } else {
        bvh_print_internal(h->l, indent + 1);
        bvh_print_internal(h->r, indent + 1);
    } printf("%.*s}\n", id, SP);
}

void bvh_print(BVH* h) {
    bvh_print_internal(h, 0);
}

//
// `Intersection` declaration

typedef struct Intersection {
    Surface s;
    double t;
} Intersection;

void intersection_print(Intersection* i) {
    printf("intersection {\n");

    switch(i->s.st) {
        case SPHERE:
            sphere_print_internal(i->s.sphere, NULL, 1);
            break;
        case TRI:
            tri_print_internal(i->s.tri, NULL, 1);
            break;
        case NONE: 
            printf("    `NONE`\n");
    }; printf("    t: %lf\n}\n", i->t);
}

Intersection helper_bvh_intersection(BVH* h, Ray r, Surface e, double t_min, double t_max) {
    Intersection intrs_a, intrs_b;
    intrs_a = (Intersection) {
        .s = (Surface) { .st = NONE },
        .t = t_max + 1.
    };

    if(!helper_bvh_ray_collides(h, r)) return intrs_a;

    if(!h->l && !h->r) {
        size_t i;
        for(i = 0; i < h->c; i++) {
            double t;
            switch(h->surfaces[i].st) {
                case TRI:
                    t = tri_intersection(*h->surfaces[i].tri, r, t_min, t_max);
                    break;
                case SPHERE:
                    t = sphere_intersection(*h->surfaces[i].sphere, r, t_min, t_max);
                    break;
                case NONE: continue;
            }

            if(t < intrs_a.t && !surface_match(e, h->surfaces[i])) {
                intrs_a.s = h->surfaces[i];
                intrs_a.t = t;
            }
        }

        return intrs_a;
    }

    intrs_a = helper_bvh_intersection(h->l, r, e, t_min, t_max);
    intrs_b = helper_bvh_intersection(h->r, r, e, t_min, t_max);

    return (intrs_a.t < intrs_b.t) ? intrs_a : intrs_b;
}

#endif /* INTRS_H */