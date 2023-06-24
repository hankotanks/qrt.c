#ifndef SCENE_H
#define SCENE_H

#include<assert.h>
#include<float.h>

#include "lalg.h"
#include "geom.h"

#define MIN(i, j) (((i) < (j)) ? (i) : (j))
#define MAX(i, j) (((i) > (j)) ? (i) : (j))

#define EPS_BVH 0.2

//
// `Camera declaration

typedef struct Camera {
    Vec pos;
    Vec at;
} Camera;

//
// `Material` declaration

typedef struct Material Material;

struct Material {
    char* name;
    Vec color_ambient;
    Vec color_diffuse;
    Vec color_spec;
    double luster;
    double metallicity;
    Material* next;
};

//
// `Config` declaration

typedef struct Config {
    double t_min;
    double t_max;
    double fov;
    double ambience;
} Config;

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
        default: break;
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
// `Scene` declaration

typedef struct Scene {
    Camera camera;
    Light* lights;
    Mesh* meshes;
    BVH* tt;
    Sphere* spheres;
} Scene;

Scene scene_new(Camera c) {
    return (Scene) {
        .camera = c,
        .lights = NULL,
        .meshes = NULL,
        .tt = NULL,
        .spheres = NULL
    };
}

void scene_add_mesh(Scene* s, Mesh* m) {
    if(s->meshes) m->next = s->meshes;

    s->meshes = m;
}

void scene_add_light(Scene* s, Light l) {
    Light* temp = malloc(sizeof *(s->lights));
    /* */ *temp = l;

    if(s->lights) temp->next = s->lights;

    s->lights = temp;
}

void scene_add_sphere(Scene* s, Sphere sphere) {
    Sphere* temp = malloc(sizeof *s->spheres);
    /*  */ *temp = sphere;

    if(s->spheres) temp->next = s->spheres;

    s->spheres = temp;
}

BVH* bvh(Mesh* meshes, Sphere* spheres);
void bvh_free(BVH* h);

void scene_initialize(Scene* s) {
    s->tt = bvh(s->meshes, s->spheres);
}

void scene_free(Scene* s) {
    if(s->tt) bvh_free(s->tt);

    Light* l;
    while(s->lights) {
        l = s->lights;
        s->lights = s->lights->next;

        free(l);
    }

    Mesh* m;
    while(s->meshes) {
        m = s->meshes;
        s->meshes = s->meshes->next;

        free(m->name);
        free(m->vertices);
        free(m->tris);
        free(m);
    }

    Sphere* sphere;
    while(s->spheres) {
        sphere = s->spheres;
        s->spheres = s->spheres->next;

        free(sphere);
    }
}

//
// `BVH` functions

void helper_bvh_extrema_point_test(Vec a, Vec* minima, Vec* maxima) {
    if(a.x < minima->x) minima->x = a.x;
    if(a.y < minima->y) minima->y = a.y;
    if(a.z < minima->z) minima->z = a.z;

    if(a.x > maxima->x) maxima->x = a.x;
    if(a.y > maxima->y) maxima->y = a.y;
    if(a.z > maxima->z) maxima->z = a.z;
}

void helper_bvh_extrema(size_t c, Surface* surfaces, Vec* minima, Vec* maxima) {
    *minima = vec_aaa(DBL_MAX);
    *maxima = vec_aaa(-1. * DBL_MAX);

    size_t i;
    for(i = 0; i < c; i++) {
        Surface s = surfaces[i];
        switch(s.st) {
            case TRI: {
                helper_bvh_extrema_point_test(s.tri->a->point, minima, maxima);
                helper_bvh_extrema_point_test(s.tri->b->point, minima, maxima);
                helper_bvh_extrema_point_test(s.tri->c->point, minima, maxima);
            }; break;
            case SPHERE: {
                Vec a = vec_aaa(s.sphere->radius);

                Vec mn = sub_vv(s.sphere->center, a);
                Vec mx = add_vv(s.sphere->center, a);

                helper_bvh_extrema_point_test(mn, minima, maxima);
                helper_bvh_extrema_point_test(mx, minima, maxima);
            }; break;
            default: break;
        }
    }
}

void bvh_split(BVH* h);

BVH* bvh(Mesh* meshes, Sphere* spheres) {
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
        default: return 0;
    }
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
// `Intersection` declaration

typedef struct Intersection {
    Surface s;
    double t;
} Intersection;

void intersection_print(Intersection* i) {
    if(i->s.st == NONE)
        printf("intersection: None\n");
    else
        printf("intersection {\n");

    switch(i->s.st) {
        case SPHERE:
            sphere_print_internal(i->s.sphere, NULL, 1);
            break;
        case TRI:
            tri_print_internal(i->s.tri, NULL, 1);
            break;
        default: return;
    }; 

    printf("    t: %lf\n", i->t);
    printf("}\n");
}

Intersection helper_bvh_intersection(Config c, BVH* h, Ray r, Surface e) {
    Intersection intrs_a, intrs_b;
    intrs_a = (Intersection) {
        .s = (Surface) { .st = NONE },
        .t = c.t_max + 1.
    };

    if(!helper_bvh_ray_collides(h, r)) return intrs_a;

    if(!h->l && !h->r) {
        size_t i;
        for(i = 0; i < h->c; i++) {
            double t;
            switch(h->surfaces[i].st) {
                case TRI:
                    t = tri_intersection(*h->surfaces[i].tri, r, c.t_min, c.t_max);
                    if(t < intrs_a.t && !(e.st == TRI && e.tri == h->surfaces[i].tri)) {
                        intrs_a.s = h->surfaces[i];
                        intrs_a.t = t;
                    }

                    break;
                case SPHERE:
                    t = sphere_intersection(*h->surfaces[i].sphere, r, c.t_min, c.t_max);
                    if(t < intrs_a.t && !(e.st == SPHERE && e.sphere == h->surfaces[i].sphere)) {
                        intrs_a.s = h->surfaces[i];
                        intrs_a.t = t;
                    }

                    break;
                default: break;
            }
        }

        return intrs_a;
    }

    intrs_a = helper_bvh_intersection(c, h->l, r, e);
    intrs_b = helper_bvh_intersection(c, h->r, r, e);

    return (intrs_a.t < intrs_b.t) ? intrs_a : intrs_b;
}

//
// Intersection check

Intersection intersection_check_excl(Scene s, Config c, Ray r, Surface e) {
    Intersection intrs_a, intrs_b;
    intrs_a = (Intersection) {
        .s = (Surface) { .st = NONE },
        .t = c.t_max + 1.
    };

    /*
    size_t i;
    for(i = 0; i < s.sc; i++) {
        double t = sphere_intersection(s.spheres[i], r, c.t_min, c.t_max);
        if(t < intrs_a.t && !(e.st == SPHERE && e.sphere == &s.spheres[i])) {
            intrs_a.s.st = SPHERE;
            intrs_a.t = t;
            intrs_a.s.sphere = &s.spheres[i];
        }
    }*/

#if 0
    Intersection intrs_c = intrs_a;
    size_t j;
    Mesh* tm; Tri* tt;
    for(i = 0; i < s.mc; i++) {
        tm = &(s.meshes[i]);
        for(j = 0; j < tm->tc; j++) {
            tt = &(tm->tris[j]);
            
            double t = tri_intersection(*tt, r, c.t_min, c.t_max);
            if(t < intrs_c.t && !(e.s == TRI && tri_eq(e.tri, tt))) {
                intrs_c.s = TRI;
                intrs_c.t = t;
                intrs_c.tri = tt;
            }
        }
    } 

    return intrs_c;
#endif

    intrs_b = helper_bvh_intersection(c, s.tt, r, e);

    return intrs_b; //(intrs_a.t < intrs_b.t) ? intrs_a : intrs_b;
}

Intersection intersection_check(Scene s, Config c, Ray r) {
    Surface e = (Surface) { .st = NONE };

    return intersection_check_excl(s, c, r, e);
}

#endif /* SCENE_H */