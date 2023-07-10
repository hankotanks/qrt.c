#include<string.h>

#include "rt.h"

//
// Main function

int main(void) {
    Camera camera = (Camera) { .pos = vec_abc(0., 10., -15.0), .at = vec_aaa(0.) };

    Scene scene = scene_new(camera);

    Material shiny_orange_temp = (Material) {
        .color_ambient = vec_abc(1., 0.4, 0.),
        .color_diffuse = vec_abc(1., 0.4, 0.),
        .color_spec = vec_abc(1., 0.4, 0.),
        .luster = 1.,
        .metallicity = 125.
    };
    Material* shiny_orange = scene_add_material(&scene, shiny_orange_temp);

    Material blue_temp = (Material) {
        .color_ambient = vec_abc(0.2, 0.2, 1.),
        .color_diffuse = vec_abc(0.2, 0.2, 1.),
        .color_spec = vec_abc(0.2, 0.2, 1.),
        .luster = 0.5,
        .metallicity = 50.
    };
    Material* blue = scene_add_material(&scene, blue_temp);

    Material muddy_green_temp = (Material) {
        .color_ambient = vec_abc(0.2, 0.4, 0.),
        .color_diffuse = vec_abc(0.2, 0.4, 0.),
        .color_spec = vec_abc(0.2, 0.4, 0.),
        .luster = 1.,
        .metallicity = 75.
    };
    Material* muddy_green = scene_add_material(&scene, muddy_green_temp);
    
    Mesh* teapot = mesh_from_raw("/mnt/c/Users/hank/Documents/projects/rt.c/models/uteapot", shiny_orange);
    mesh_transform(teapot, transform_rotate(Z, 1.570796));

    scene_add_mesh(&scene, teapot, STATIC);
    scene_add_light(&scene, light_new(vec_abc(15., 10., 0.), 0.8));
    scene_add_light(&scene, light_new(vec_abc(-15., 10., 0.), 0.8));

    scene_add_sphere(&scene, (Sphere) {
        .center = vec_abc(0.0, 0.0, 15.0), 
        .radius = 10.,
        .material = blue 
    }, STATIC);

    Sphere* dyn_sphere = scene_add_sphere(&scene, (Sphere) {
       .center = vec_abc(8., -8., 6.),
       .radius = 4.,
       .material = muddy_green
    }, DYNAMIC);

    scene_initialize(&scene);

    Config c = (Config) {
        .t_min = 0.01,
        .t_max = 1000.,
        .fov = 1.570796,
        .ambience = 0.2
    };
    
    Buffer b = buffer_wh(640, 360);

    raytrace(b, scene, c);

    buffer_export_as_ppm(b, "test.ppm");

    printf("Complete...\n");

    scene_free(&scene);
    buffer_free(&b);

    return 0;
}

/*
 * TODO:
 *  - `surfaces` only need to be calculated when `scene_add_sphere` and `scene_add_mesh` are called.
 *    `tt` needs to be calculated whenever `surfaces` does, but also when objects move.
 *     - I should implement a system to manage this.
 *  - I'll need to double check BVH and `surfaces` state when reinitializing.
 */