#ifndef INTERSECTION_H
#define INTERSECTION_H

#include "lalg.h"
#include "geom.h"

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
    switch(i->s) {
        case SPHERE: {
            sphere_print(i->sphere);
            break;
        }
        case TRI: {
            tri_print(i->tri);
            break;
        }
        default: printf("no intersection...");
    };
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
            if(t < intrs.t && !(e.s == TRI && e.tri != tt)) {
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