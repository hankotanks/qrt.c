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
    SLL* materials;
    BVH* tt;
    SLL* lights;
    SLL* meshes;
    SLL* spheres;    
    size_t sc;
    Surface* surfaces;
} Scene;

Scene scene_new(Camera c) {
    return (Scene) {
        .camera = c,
        .materials = NULL,
        .tt = NULL,
        .lights = NULL,
        .meshes = NULL,
        .spheres = NULL,
        .surfaces = NULL
    };
}

void scene_add_mesh(Scene* s, Mesh* mesh) {
    s->meshes = sll_insert(s->meshes, mesh);
}

Light* scene_add_light(Scene* s, Light temp) {
    Light* light = malloc(sizeof *light);
    memcpy(light, &temp, sizeof *light);

    s->lights = sll_insert(s->lights, light);

    return light;
}

Material* scene_add_material(Scene* s, Material temp) {
    Material* material = malloc(sizeof *material);
    memcpy(material, &temp, sizeof *material);

    s->materials = sll_insert(s->materials, material);

    return material;
}

Sphere* scene_add_sphere(Scene* s, Sphere temp) {
    Sphere* sphere = malloc(sizeof *sphere);
    memcpy(sphere, &temp, sizeof *sphere);

    s->spheres = sll_insert(s->spheres, sphere);

    return sphere;
}

void scene_initialize(Scene* s) {
    assert(s->meshes || s->spheres);

    SLL* curr;

    size_t sc = 0, t = 0;
    for(curr = s->meshes; curr; curr = curr->next) 
        sc += ((Mesh*) curr->item)->tc;
    for(curr = s->spheres; curr; curr = curr->next) ++sc;

    Surface* surfaces = malloc(sc * sizeof *surfaces);
    for(curr = s->meshes; curr; curr = curr->next) {
        Mesh* mesh = (Mesh*) curr->item;

        size_t i;
        for(i = 0; i < mesh->tc; i++) {
            surfaces[t] = (Surface) {
                .st = TRI,
                .tri = &mesh->tris[i]
            }; t++;
        }
    }

    for(curr = s->spheres; curr; curr = curr->next) {
        surfaces[t] = (Surface) {
            .st = SPHERE,
            .sphere = (Sphere*) curr->item
        }; t++;
    }

    s->sc = sc;
    s->surfaces = surfaces;
    s->tt = bvh_initialize(s->sc, s->surfaces);
}

void scene_free(Scene* s) {
    SLL* temp;

    while(s->materials) {
        temp = s->materials;
        s->materials = s->materials->next;

        Material* item = (Material*) temp->item;
        free(item);
        free(temp);
    }
    
    while(s->lights) {
        temp = s->lights;
        s->lights = s->lights->next;

        Light* item = (Light*) temp->item;
        free(item);
        free(temp);
    }

    while(s->meshes) {
        temp = s->meshes;
        s->meshes = s->meshes->next;

        Mesh* item = (Mesh*) temp->item;
        free(item->tris);
        free(item);
        free(temp);
    }

    while(s->spheres) {
        temp = s->spheres;
        s->spheres = s->spheres->next;

        Sphere* item = (Sphere*) temp->item;
        free(item);
        free(temp);
    }

    if(s->tt) bvh_free(s->tt);

    if(s->surfaces) free(s->surfaces);
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