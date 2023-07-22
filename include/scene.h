#ifndef SCENE_H
#define SCENE_H

#include<assert.h>
#include<float.h>
#include<string.h>

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
    size_t block_size;
    size_t threads;
} Config;

//
// `ObjectMode` declaration

typedef enum Motility { STATIC = 0, DYNAMIC } Motility;

//
// `Scene` declaration

typedef struct Scene {
    Camera camera;
    SLL* materials;
    BVH* tt;
    SLL* lights;
    SLL* s_meshes;
    SLL* d_meshes;
    SLL* s_spheres;   
    SLL* d_spheres; 
    size_t ssc;
    Surface* s_surfaces;
    size_t dsc;
    Surface* d_surfaces;
} Scene;

Scene scene_new(Camera c) {
    return (Scene) {
        .camera = c,
        .materials = NULL,
        .tt = NULL,
        .lights = NULL,
        .s_meshes = NULL,
        .d_meshes = NULL,
        .s_spheres = NULL,
        .d_spheres = NULL,
        .s_surfaces = NULL,
        .d_surfaces = NULL
    };
}

Mesh* scene_add_mesh(Scene* s, Mesh temp, Motility om) {
    Mesh* mesh = malloc(sizeof *mesh);
    memcpy(mesh, &temp, sizeof *mesh);

    if(om)
        s->d_meshes = sll_insert(s->d_meshes, mesh);
    else 
        s->s_meshes = sll_insert(s->s_meshes, mesh);

    return mesh;
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

Sphere* scene_add_sphere(Scene* s, Sphere temp, Motility om) {
    Sphere* sphere = malloc(sizeof *sphere);
    memcpy(sphere, &temp, sizeof *sphere);

    if(om)
        s->d_spheres = sll_insert(s->d_spheres, sphere);
    else 
        s->s_spheres = sll_insert(s->s_spheres, sphere);

    return sphere;
}

void helper_scene_surface_init(SLL* meshes, SLL* spheres, Surface** surfaces, size_t* sc) {
    SLL* curr;

    size_t t = 0;
    for(curr = meshes; curr; curr = curr->next) 
        *sc += ((Mesh*) curr->item)->tc;
    for(curr = spheres; curr; curr = curr->next) (*sc)++;

    *surfaces = malloc(*sc * sizeof **surfaces);
    for(curr = meshes; curr; curr = curr->next) {
        Mesh* mesh = (Mesh*) curr->item;

        size_t i;
        for(i = 0; i < mesh->tc; i++) {
            (*surfaces)[t] = (Surface) {
                .st = TRI,
                .tri = &mesh->tris[i]
            }; t++;
        }
    }

    for(curr = spheres; curr; curr = curr->next) {
        (*surfaces)[t] = (Surface) {
            .st = SPHERE,
            .sphere = (Sphere*) curr->item
        }; t++;
    }
}

void scene_initialize(Scene* s) {
    assert(!s->tt &&
        "Error: BVH has been previously initialized");
    assert((!s->s_surfaces && !s->d_surfaces) &&
        "Error: Surface arrays have already been populated");
    assert((s->s_meshes || s->d_meshes || s->s_spheres || s->d_spheres) &&
        "Error: The provided Scene has no drawable objects");

    helper_scene_surface_init(s->s_meshes, s->s_spheres, &s->s_surfaces, &s->ssc);
    helper_scene_surface_init(s->d_meshes, s->d_spheres, &s->d_surfaces, &s->dsc);

    s->tt = bvh_initialize(s->ssc, s->s_surfaces);
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

    while(s->s_meshes) {
        temp = s->s_meshes;
        s->s_meshes = s->s_meshes->next;

        Mesh* item = (Mesh*) temp->item;
        free(item->tris);
        free(item);
        free(temp);
    }

    while(s->d_meshes) {
        temp = s->d_meshes;
        s->d_meshes = s->d_meshes->next;

        Mesh* item = (Mesh*) temp->item;
        free(item->tris);
        free(item);
        free(temp);
    }

    while(s->s_spheres) {
        temp = s->s_spheres;
        s->s_spheres = s->s_spheres->next;

        Sphere* item = (Sphere*) temp->item;
        free(item);
        free(temp);
    }

    while(s->d_spheres) {
        temp = s->d_spheres;
        s->d_spheres = s->d_spheres->next;

        Sphere* item = (Sphere*) temp->item;
        free(item);
        free(temp);
    }

    if(s->tt) bvh_free(s->tt);

    if(s->s_surfaces) free(s->s_surfaces);
    if(s->d_surfaces) free(s->d_surfaces);
}

//
// Intersection check

Intersection intersection_check_excl(Scene s, Config c, Ray r, Surface e) {
    Intersection intrs = helper_bvh_intersection(s.tt, r, e, c.t_min, c.t_max);

    size_t i;
    for(i = 0; i < s.dsc; i++) {
        Surface sf = s.d_surfaces[i];

        double t;
        switch(sf.st) {
            case TRI:
                t = tri_intersection(*sf.tri, r, c.t_min, c.t_max);
                break;
            case SPHERE:
                t = sphere_intersection(*sf.sphere, r, c.t_min, c.t_max);
                break;
            case NONE: continue;
        }

        if(t < intrs.t && !surface_match(e, sf)) {
            intrs.s = sf;
            intrs.t = t;
        }
    }

    return intrs;
}

Intersection intersection_check(Scene s, Config c, Ray r) {
    Surface e = (Surface) { .st = NONE };

    return intersection_check_excl(s, c, r, e);
}

#endif /* SCENE_H */