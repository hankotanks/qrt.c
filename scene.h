#ifndef SCENE_H
#define SCENE_H

#include<assert.h>
#include<float.h>

#include "geom.h"
#include "intrs.h"

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

void scene_initialize(Scene* s) {
    s->tt = bvh_initialize(s->meshes, s->spheres);
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
// Intersection check

Intersection intersection_check_excl(Scene s, Config c, Ray r, Surface e) {
    return helper_bvh_intersection(s.tt, r, e, c.t_min, c.t_max);
}

Intersection intersection_check(Scene s, Config c, Ray r) {
    Surface e = (Surface) { .st = NONE };

    return intersection_check_excl(s, c, r, e);
}

#endif /* SCENE_H */