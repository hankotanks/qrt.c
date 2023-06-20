#ifndef INTERSECTION_H
#define INTERSECTION_H

#include<assert.h>

#include "lalg.h"
#include "geom.h"

#define EPS_BVH 0.000002

//
// `Camera declaration

typedef struct Camera {
    Vec pos;
    Vec at;
} Camera;

//
// `Scene` declaration

typedef struct Scene {
    Camera camera;
    size_t mc;
    size_t sc;
    size_t lc;
    Mesh* meshes;
    Sphere* spheres;
    Light* lights;
} Scene;

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

typedef enum Surface { NONE = 0, TRI, SPHERE } Surface;

//
// `Exclusion` declaration

typedef struct Exclusion {
    Surface s;
    union {
        Tri* tri;
        Sphere* sphere;
    };
} Exclusion;

void exclusion_print(Exclusion* e) {
    if(e->s == NONE)
        printf("exclusion: None\n");
    else
        printf("exclusion {\n");

    switch(e->s) {
        case SPHERE:
            sphere_print_internal(e->sphere, NULL, 1);
            break;
        case TRI:
            tri_print_internal(e->tri, NULL, 1);
            break;
        default: return;
    }; printf("}\n");
}

//
// `BVH` declaration

typedef struct BVH BVH;

struct BVH {
    BVH* l;
    BVH* r;
    Vec minima;
    Vec maxima;
    size_t tc;
    Tri* tris;
};

void bvh_split(BVH* h);

BVH bvh(size_t tc, Tri* tris) {
    if(!tc) return (BVH) {  };

    Vec minima = tris[0].centroid;
    Vec maxima = minima;

    Vec c;

    size_t i;
    for(i = 1; i < tc; i++) {
        c = tris[i].centroid;

        if(c.x < minima.x) minima.x = c.x;
        if(c.y < minima.y) minima.y = c.y;
        if(c.z < minima.z) minima.z = c.z;

        if(c.x > maxima.x) maxima.x = c.x;
        if(c.y > maxima.y) maxima.y = c.y;
        if(c.z > maxima.z) maxima.z = c.z;
    }

    BVH h = (BVH) {
        .l = NULL,
        .r = NULL,
        .minima = minima,
        .maxima = maxima,
        .tc = tc,
        .tris = tris
    };

    bvh_split(&h);

    return h;
}

//
// Helper functions

int helper_bvh_contains(Vec minima, Vec maxima, Vec t) {
    return (
           t.x >= minima.x
        && t.x <= maxima.x
        && t.y >= minima.y
        && t.y <= maxima.y
        && t.z >= minima.z
        && t.z <= maxima.z
    );
}

int helper_bvh_scale_check(double d, BVH* l, BVH* r) {
    if(d < EPS_BVH) {
        free(l->tris);
        free(l);
        free(r->tris);
        free(r);

        return 1;
    }

    return 0;
}

//
// `BVH` split functionality

void bvh_split(BVH* h) {
    if(h->tc == 1) return;

    double dx, dy, dz;
    dx = h->maxima.x - h->minima.x;
    dy = h->maxima.y - h->minima.y;
    dz = h->maxima.z - h->minima.z;

    BVH* l = malloc(sizeof *l);
    *l = (BVH) {
        .minima = h->minima,
        .maxima = h->maxima,
        .tris = malloc(h->tc * (sizeof *(l->tris)))
    };

    BVH* r = malloc(sizeof *r);
    *r = (BVH) {
        .minima = h->minima,
        .maxima = h->maxima,
        .tris = malloc(h->tc * (sizeof *(r->tris)))
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
    for(i = 0; i < h->tc; i++) {
        if(helper_bvh_contains(l->minima, l->maxima, h->tris[i].centroid)) {
            l->tris[l->tc] = h->tris[i];
            ++(l->tc);
        } else {
            r->tris[r->tc] = h->tris[i];
            ++(r->tc);
        }
    }

    if(l->tc == 0) {
        h->minima = r->minima;
        h->maxima = r->maxima;

        helper_bvh_scale_check(0., l, r);

        bvh_split(h);
    } else if(r->tc == 0) {
        h->minima = l->minima;
        h->maxima = l->maxima;

        helper_bvh_scale_check(0., l, r);

        bvh_split(h);
    } else {
        l->tris = realloc(l->tris, l->tc * (sizeof *(l->tris)));
        assert(l->tris);

        r->tris = realloc(r->tris, r->tc * (sizeof *(r->tris)));
        assert(r->tris);
        
        bvh_split(l);
        bvh_split(r);

        h->l = l;
        h->r = r;
    }
}

void bvh_print(BVH* h) {
    // TODO
}

//
// `Intersection` declaration

typedef struct Intersection {
    Surface s;
    double t;
    union {
        Tri* tri;
        Sphere* sphere;
    };
} Intersection;

void intersection_print(Intersection* i) {
    if(i->s == NONE)
        printf("intersection: None\n");
    else
        printf("intersection {\n");

    switch(i->s) {
        case SPHERE:
            sphere_print_internal(i->sphere, NULL, 1);
            break;
        case TRI:
            tri_print_internal(i->tri, NULL, 1);
            break;
        default: return;
    }; printf("}\n");
}

//
// Intersection check

Intersection intersection_check_excl(Scene s, Config c, Ray r, Exclusion e) {
    Intersection intrs = (Intersection) {
        .s = NONE,
        .t = c.t_max + 1.
    };

    size_t i, j;

    double t;
    for(i = 0; i < s.sc; i++) {
        t = sphere_intersection(s.spheres[i], r, c.t_min, c.t_max);
        if(t < intrs.t && !(e.s == SPHERE && e.sphere != &(s.spheres[i]))) {
            intrs.s = SPHERE;
            intrs.t = t;
            intrs.sphere = &(s.spheres[i]);
        }
    }

    Mesh* tm; Tri* tt;
    for(i = 0; i < s.mc; i++) {
        tm = &(s.meshes[i]);
        for(j = 0; j < tm->tc; j++) {
            tt = &(tm->tris[j]);
            
            t = tri_intersection(*tt, r, c.t_min, c.t_max);
            if(t < intrs.t && !(e.s == TRI && tri_eq(e.tri, tt))) {
                intrs.s = TRI;
                intrs.t = t;
                intrs.tri = tt;
            }
        }
    }

    return intrs;
}

Intersection intersection_check(Scene s, Config c, Ray r) {
    Exclusion e = (Exclusion) { .s = NONE };

    return intersection_check_excl(s, c, r, e);
}

#endif /* INTERSECTION_H */