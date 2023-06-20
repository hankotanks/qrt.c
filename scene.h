#ifndef SCENE_H
#define SCENE_H

#include<assert.h>
#include<float.h>

#include "lalg.h"
#include "geom.h"

#define MIN(i, j) (((i) < (j)) ? (i) : (j))
#define MAX(i, j) (((i) > (j)) ? (i) : (j))

//
// `Camera declaration

typedef struct Camera {
    Vec pos;
    Vec at;
} Camera;

//
// `Config` declaration

typedef struct Config {
    double t_min;
    double t_max;
    double fov;
    double ambience;
} Config;

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

//
// `Scene` declaration

typedef struct Scene {
    Camera camera;
    size_t mc;
    size_t sc;
    size_t lc;
    Mesh* meshes;
    BVH* hierarchy;
    Sphere* spheres;
    Light* lights;
} Scene;

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
// `BVH` functions

void bvh_split(BVH* h);

BVH* bvh(size_t mc, Mesh* meshes) {
    size_t m, tc = 0;
    for(m = 0; m < mc; m++) tc += meshes[m].tc;

    Tri* tris = malloc(tc * (sizeof *tris));

    size_t n, ctc = 0;
    for(m = 0; m < mc; m++) {
        for(n = 0; n < meshes[m].tc; n++) {
            tris[ctc] = meshes[m].tris[n];
            ++ctc;
        }
    }

    if(!tc) return NULL;

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

    BVH* h = malloc(sizeof *h);
    *h = (BVH) {
        .l = NULL,
        .r = NULL,
        .minima = minima,
        .maxima = maxima,
        .tc = tc,
        .tris = tris
    };

    bvh_split(h);

    return h;
}

void bvh_free(BVH* h) {
    if(h->l) bvh_free(h->l);
    if(h->r) bvh_free(h->r);
    
    free(h->tris);
    free(h);
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
    if(d < 0.000002) {
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
        .l = NULL,
        .r = NULL,
        .minima = h->minima,
        .maxima = h->maxima,
        .tc = 0,
        .tris = malloc(h->tc * (sizeof *(l->tris)))
    };

    BVH* r = malloc(sizeof *r);
    *r = (BVH) {
        .l = NULL,
        .r = NULL,
        .minima = h->minima,
        .maxima = h->maxima,
        .tc = 0,
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

int helper_bvh_ray_collides(BVH* h, Ray r) {
    double min = 0.0;
    double max = DBL_MAX;
    
    double t0, t1;
    t0 = (h->minima.x - r.origin.x) / r.dir.x;
    t1 = (h->maxima.x - r.origin.x) / r.dir.x;

    min = MAX(min, MIN(t0, t1));
    max = MIN(max, MAX(t0, t1)); 

    t0 = (h->minima.y - r.origin.y) / r.dir.y;
    t1 = (h->maxima.y - r.origin.y) / r.dir.y;

    min = MAX(min, MIN(t0, t1));
    max = MIN(max, MAX(t0, t1)); 

    t0 = (h->minima.z - r.origin.z) / r.dir.z;
    t1 = (h->maxima.z - r.origin.z) / r.dir.z;

    min = MAX(min, MIN(t0, t1));
    max = MIN(max, MAX(t0, t1)); 

    return min <= max;
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

Intersection helper_bvh_intersection(Config c, BVH* h, Ray r, Exclusion e) {
    Intersection intrs = (Intersection) {
        .s = NONE,
        .t = c.t_max + 1.
    };

    if(!helper_bvh_ray_collides(h, r)) return intrs;

    if(!h->l || !h->r) {
        size_t i; double t; Tri* temp;
        for(i = 0; i < h->tc; i++) {
            temp = &(h->tris[i]);
            t = tri_intersection(*temp, r, c.t_min, c.t_max);

            if(t < intrs.t && !(e.s == TRI && tri_eq(e.tri, temp))) {
                intrs.s = TRI;
                intrs.t = t;
                intrs.tri = temp;
            }
        }

        return intrs;
    }

    Intersection left, right;
    left = helper_bvh_intersection(c, h->l, r, e);
    right = helper_bvh_intersection(c, h->r, r, e);

    return (left.t <= right.t) ? left : right;
}

//
// Intersection check

Intersection intersection_check_excl(Scene s, Config c, Ray r, Exclusion e) {
    Intersection intrs_a, intrs_b;
    intrs_a = (Intersection) {
        .s = NONE,
        .t = c.t_max + 1.
    };

    size_t i; double t;
    for(i = 0; i < s.sc; i++) {
        t = sphere_intersection(s.spheres[i], r, c.t_min, c.t_max);
        if(t < intrs_a.t && !(e.s == SPHERE && e.sphere != &(s.spheres[i]))) {
            intrs_a.s = SPHERE;
            intrs_a.t = t;
            intrs_a.sphere = &(s.spheres[i]);
        }
    }

    /*
    size_t j;
    Mesh* tm; Tri* tt;
    for(i = 0; i < s.mc; i++) {
        tm = &(s.meshes[i]);
        for(j = 0; j < tm->tc; j++) {
            tt = &(tm->tris[j]);
            
            t = tri_intersection(*tt, r, c.t_min, c.t_max);
            if(t < intrs_a.t && !(e.s == TRI && tri_eq(e.tri, tt))) {
                intrs_a.s = TRI;
                intrs_a.t = t;
                intrs_a.tri = tt;
            }
        }
    } 

    printf("%lf\n", intrs_a.t);

    return intrs_a;
    */

    intrs_b = helper_bvh_intersection(c, s.hierarchy, r, e);

    return (intrs_b.t < intrs_a.t) ? intrs_b : intrs_a;
}

Intersection intersection_check(Scene s, Config c, Ray r) {
    Exclusion e = (Exclusion) { .s = NONE };

    return intersection_check_excl(s, c, r, e);
}

#endif /* SCENE_H */