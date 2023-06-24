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
    size_t sc;
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

BVH* bvh(Mesh* meshes);
void bvh_free(BVH* h);

void scene_initialize(Scene* s) {
    s->tt = bvh(s->meshes);
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


    //if(s->lights) free(s->lights);
    if(s->spheres) free(s->spheres);
}

//
// `Surface` declaration

typedef enum Surface { NONE = 0, TRI, SPHERE } Surface;

//
// `Exclusion` declaration

typedef struct Exclusion {
    Surface s;
    union {
        Tri tri;
        Sphere sphere;
    };
} Exclusion;

void exclusion_print(Exclusion* e) {
    if(e->s == NONE)
        printf("exclusion: None\n");
    else
        printf("exclusion {\n");

    switch(e->s) {
        case SPHERE:
            sphere_print_internal(&e->sphere, NULL, 1);
            break;
        case TRI:
            tri_print_internal(&e->tri, NULL, 1);
            break;
        default: return;
    }; printf("}\n");
}

//
// `BVH` functions

void helper_bvh_extrema_test(Tri t, Vec* minima, Vec* maxima) {
    Vec a = t.a->point;
    Vec b = t.b->point;
    Vec c = t.c->point;

    if(a.x < minima->x) minima->x = a.x;
    if(a.y < minima->y) minima->y = a.y;
    if(a.z < minima->z) minima->z = a.z;

    if(a.x > maxima->x) maxima->x = a.x;
    if(a.y > maxima->y) maxima->y = a.y;
    if(a.z > maxima->z) maxima->z = a.z;

    if(b.x < minima->x) minima->x = b.x;
    if(b.y < minima->y) minima->y = b.y;
    if(b.z < minima->z) minima->z = b.z;

    if(b.x > maxima->x) maxima->x = b.x;
    if(b.y > maxima->y) maxima->y = b.y;
    if(b.z > maxima->z) maxima->z = b.z;

    if(c.x < minima->x) minima->x = c.x;
    if(c.y < minima->y) minima->y = c.y;
    if(c.z < minima->z) minima->z = c.z;

    if(c.x > maxima->x) maxima->x = c.x;
    if(c.y > maxima->y) maxima->y = c.y;
    if(c.z > maxima->z) maxima->z = c.z;
}

void helper_bvh_extrema(size_t tc, Tri* tris, Vec* minima, Vec* maxima) {
    *minima = vec_aaa(DBL_MAX);
    *maxima = vec_aaa(-1. * DBL_MAX);

    size_t i;
    for(i = 0; i < tc; i++)
        helper_bvh_extrema_test(tris[i], minima, maxima);
}

void bvh_split(BVH* h);

BVH* bvh(Mesh* meshes) {
    if(!meshes) return NULL;

    Mesh* curr = meshes;

    size_t tc = 0, c = 0;
    while(curr != NULL) {
        tc += curr->tc;
        curr = curr->next;
    }
    
    if(!tc) return NULL;

    Tri* tris = malloc(tc * (sizeof *tris));

    curr = meshes;
    while(curr != NULL) {
        size_t n;
        for(n = 0; n < curr->tc; n++) {
            tris[c] = curr->tris[n];
            ++c;
        }

        curr = curr->next;
    }

    Vec minima, maxima;
    helper_bvh_extrema(tc, tris, &minima, &maxima);

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

int helper_bvh_contains(Vec minima, Vec maxima, Tri t) {
    return helper_bvh_contains_point(minima, maxima, t.centroid);
}

int helper_bvh_scale_check(double d, BVH* l, BVH* r) {
    if(d < 0.5 * EPS_BVH) { // 0.000002
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
        if(helper_bvh_contains(l->minima, l->maxima, h->tris[i])) {
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

        helper_bvh_extrema(l->tc, l->tris, &l->minima, &l->maxima);
        bvh_split(l);

        helper_bvh_extrema(r->tc, r->tris, &r->minima, &r->maxima);
        bvh_split(r);

        h->l = l;
        h->r = r;
    }
}

void bvh_print_internal(BVH* h, size_t indent) {
    int id = 4 * (int) indent;

    printf("%.*sbvh (%u) {\n", id, SP, (unsigned) h->tc);

    if(!h->l && !h->r) {
        size_t i;
        for(i = 0; i < h->tc; i++) 
            tri_print_internal(&h->tris[i], NULL, indent + 1);
    } else {
        bvh_print_internal(h->l, indent + 1);
        bvh_print_internal(h->r, indent + 1);
    }

    printf("%.*s}\n", id, SP);
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
    union {
        Tri tri;
        Sphere sphere;
    };
} Intersection;

void intersection_print(Intersection* i) {
    if(i->s == NONE)
        printf("intersection: None\n");
    else
        printf("intersection {\n");

    switch(i->s) {
        case SPHERE:
            sphere_print_internal(&i->sphere, NULL, 1);
            break;
        case TRI:
            tri_print_internal(&i->tri, NULL, 1);
            break;
        default: return;
    }; 

    printf("    t: %lf\n", i->t);
    printf("}\n");
}

Intersection helper_bvh_intersection(Config c, BVH* h, Ray r, Exclusion e) {
    Intersection intrs_a, intrs_b;
    intrs_a = (Intersection) {
        .s = NONE,
        .t = c.t_max + 1.
    };

    if(!helper_bvh_ray_collides(h, r)) return intrs_a;

    if(!h->l && !h->r) {
        size_t i;
        for(i = 0; i < h->tc; i++) {
            double t = tri_intersection(h->tris[i], r, c.t_min, c.t_max);
            if(t < intrs_a.t && !(e.s == TRI && tri_eq(e.tri, h->tris[i]))) {
                intrs_a.s = TRI;
                intrs_a.t = t;
                intrs_a.tri = h->tris[i];
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

Intersection intersection_check_excl(Scene s, Config c, Ray r, Exclusion e) {
    Intersection intrs_a, intrs_b;
    intrs_a = (Intersection) {
        .s = NONE,
        .t = c.t_max + 1.
    };

    size_t i;
    for(i = 0; i < s.sc; i++) {
        double t = sphere_intersection(s.spheres[i], r, c.t_min, c.t_max);
        if(t < intrs_a.t && !(e.s == SPHERE && sphere_eq(e.sphere, s.spheres[i]))) {
            intrs_a.s = SPHERE;
            intrs_a.t = t;
            intrs_a.sphere = s.spheres[i];
        }
    }

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

    return (intrs_a.t < intrs_b.t) ? intrs_a : intrs_b;
}

Intersection intersection_check(Scene s, Config c, Ray r) {
    Exclusion e = (Exclusion) { .s = NONE };

    return intersection_check_excl(s, c, r, e);
}

Exclusion intersection_into_exclusion(Intersection i) {
    Exclusion e = (Exclusion) { .s = i.s };
    switch(i.s) {
        case SPHERE: 
            e.sphere = i.sphere;
            break;
        case TRI:
            e.tri = i.tri;
            break;
        default: break;
    }

    return e;
}

#endif /* SCENE_H */