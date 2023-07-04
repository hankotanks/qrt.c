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
    Material* materials;
    BVH* tt;
    Light* lights;
    Mesh* meshes;
    Sphere* spheres;    
} Scene;

Scene scene_new(Camera c) {
    return (Scene) {
        .camera = c,
        .materials = NULL,
        .tt = NULL,
        .lights = NULL,
        .meshes = NULL,
        .spheres = NULL
    };
}

void scene_add_mesh(Scene* s, Mesh* m) {
    if(s->meshes) m->next = s->meshes;

    s->meshes = m;
}

Light* scene_add_light(Scene* s, Light l) {
    Light* temp = malloc(sizeof *(s->lights));
    /* */ *temp = l;

    if(s->lights) temp->next = s->lights;

    s->lights = temp;

    return s->lights;
}

Material* scene_add_material(Scene* s, Material material) {
    Material* temp = malloc(sizeof *s->materials);
    /*     */*temp = material;

    if(s->materials) temp->next = s->materials;

    s->materials = temp;

    return s->materials;
}

Sphere* scene_add_sphere(Scene* s, Sphere sphere) {
    Sphere* temp = malloc(sizeof *s->spheres);
    /*  */ *temp = sphere;

    if(s->spheres) temp->next = s->spheres;

    s->spheres = temp;

    return s->spheres;
}

void scene_initialize(Scene* s) {
    s->tt = bvh_initialize(s->meshes, s->spheres);
}

void scene_free(Scene* s) {
    Material* material;
    while(s->materials) {
        material = s->materials;
        s->materials = s->materials->next;

        free(material->name);
        free(material);
    }

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