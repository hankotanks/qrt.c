#ifndef INTRS_H
#define INTRS_H

#include<assert.h>
#include<float.h>

#include "geom.h"

#define EPS_BVH 0.2

//
// `SLL` declaration

typedef struct SLL SLL;

struct SLL {
    void* item;
    SLL* next;
};

//
// Add to `SLL` function

SLL* sll_insert(SLL* head, void* item) {
    SLL node_temp = (SLL) {
        .item = item,
        .next = head
    };

    SLL* node = malloc(sizeof *node);
    memcpy(node, &node_temp, sizeof *node);

    return node;
}

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
        printf("%.*s%s (surface) {\n", id, PADDING, name);
    else 
        printf("%.*s surface {\n", id, PADDING);

    switch(s->st) {
        case TRI:
            tri_print_internal(s->tri, NULL, indent + 1);
            break;
        case SPHERE:
            sphere_print_internal(s->sphere, NULL, indent + 1);
            break;
        case NONE: 
            printf("%.*s`NONE`\n", id + 4, PADDING);
    } printf("%.*s}\n", id, PADDING);
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
    SLL* surfaces;
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

void helper_bvh_extrema(SLL* surfaces, Vec* minima, Vec* maxima) {
    *minima = vec_aaa(DBL_MAX);
    *maxima = vec_aaa(-1. * DBL_MAX);

    SLL* curr;
    for(curr = surfaces; curr; curr = curr->next) {
        Surface s = *(Surface*) curr->item;

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

// Forward definition of the hierarchy split functionality
void bvh_split(BVH* h);

BVH* bvh_initialize(size_t sc, Surface* surfaces) {
    SLL* head = NULL;
    
    size_t i;
    for(i = 0; i < sc; i++) head = sll_insert(head, &surfaces[i]);

    Vec minima, maxima;
    helper_bvh_extrema(head, &minima, &maxima);

    BVH* h = malloc(sizeof *h);
    *h = (BVH) {
        .l = NULL,
        .r = NULL,
        .minima = minima,
        .maxima = maxima, // `sc` is initalized to 0 implicitly
        .surfaces = head
    };
    
    bvh_split(h);
    
    return h;
}

void bvh_free(BVH* h) {
    if(!h) return;
    
    if(h->l) bvh_free(h->l);
    if(h->r) bvh_free(h->r);

    SLL* temp;
    while(h->surfaces) {
        temp = h->surfaces;
        h->surfaces = h->surfaces->next;

        free(temp);
    }
    
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
        free(l);
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
    if(!h->surfaces->next) return;

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
        .surfaces = NULL
    };

    BVH* r = malloc(sizeof *r);
    *r = (BVH) {
        .l = NULL,
        .r = NULL,
        .minima = h->minima,
        .maxima = h->maxima,
        .surfaces = NULL
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

    SLL* curr;
    for(curr = h->surfaces; curr; curr = curr->next) {
        Surface* s = (Surface*) curr->item;

        if(helper_bvh_contains(l->minima, l->maxima, *s))
            l->surfaces = sll_insert(l->surfaces, s);
        else
            r->surfaces = sll_insert(r->surfaces, s);
    }

    if(!l->surfaces) {
        h->minima = r->minima;
        h->maxima = r->maxima;

        helper_bvh_scale_check(0., l, r);

        bvh_split(h);
    } else if(!r->surfaces) {
        h->minima = l->minima;
        h->maxima = l->maxima;

        helper_bvh_scale_check(0., l, r);

        bvh_split(h);
    } else {
        assert(l->surfaces && r->surfaces);

        helper_bvh_extrema(l->surfaces, &l->minima, &l->maxima);
        bvh_split(l);

        helper_bvh_extrema(r->surfaces, &r->minima, &r->maxima);
        bvh_split(r);

        h->l = l;
        h->r = r;
    }
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

Vec helper_intersection_tri_normal(Tri t, Vec pos) {
    Vec a, b, c;
    a = t.a.point;
    b = t.b.point;
    c = t.c.point;
    
    Vec v0, v1, v2;
    v0 = sub_vv(b, a);
    v1 = sub_vv(c, a);
    v2 = sub_vv(pos, a);

    double d00, d01, d11, d20, d21;
    d00 = dot_vv(v0, v0);
    d01 = dot_vv(v0, v1);
    d11 = dot_vv(v1, v1);
    d20 = dot_vv(v2, v0);
    d21 = dot_vv(v2, v1);
    
    double denom = d00 * d11 - d01 * d01;

    double v, w, u;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1. - v - w;

    Vec na, nb, nc;
    na = mul_vs(t.a.normal, v);
    nb = mul_vs(t.b.normal, w);
    nc = mul_vs(t.c.normal, u);

    return add_vv(add_vv(na, nb), nc);
}

void intersection_normal(Intersection i, Ray r, Vec* normal, Vec* hit) {
    assert(i.s.st);

    *hit = add_vv(r.origin, mul_vs(r.dir, i.t));

    switch(i.s.st) {
        case SPHERE: 
            *normal = norm_v(sub_vv(*hit, i.s.sphere->center));
            break;
        case TRI: 
            *normal = helper_intersection_tri_normal(*i.s.tri, *hit);
            break;
        case NONE:
            assert(0);
    }
}

Material* intersection_material(Intersection i) {
    assert(i.s.st);

    Material* material;
    switch(i.s.st) {
        case SPHERE:
            material = i.s.sphere->material;
            break;
        case TRI:
            material = i.s.tri->material;
            break;
        case NONE:
            assert(0);
    }

    return material;
}

Intersection helper_bvh_intersection(BVH* h, Ray r, Surface e, double t_min, double t_max) {
    Intersection intrs_a, intrs_b;
    intrs_a = (Intersection) {
        .s = (Surface) { .st = NONE },
        .t = t_max + 1.
    };

    if(!helper_bvh_ray_collides(h, r)) return intrs_a;

    if(!h->l && !h->r) {
        SLL* curr;
        for(curr = h->surfaces; curr; curr = curr->next) {
            Surface s = *(Surface*) curr->item;

            double t;
            switch(s.st) {
                case TRI:
                    t = tri_intersection(*s.tri, r, t_min, t_max);
                    break;
                case SPHERE:
                    t = sphere_intersection(*s.sphere, r, t_min, t_max);
                    break;
                case NONE: continue;
            }

            if(t < intrs_a.t && !surface_match(e, s)) {
                intrs_a.s = s;
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