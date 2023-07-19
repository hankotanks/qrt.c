#include<string.h>

#include "rt.h"

#define TEAPOT "C:/Users/hank/Documents/projects/rt.c/models/uteapot"

//
// Main function

int main(void) {
    Config config = (Config) {
        .t_min = 0.01,
        .t_max = 1000.,
        .fov = 1.570796,
        .ambience = 0.2,
        .block_size = 10,
        .threads = 8
    };

    Camera camera = (Camera) { 
        .pos = vec_abc(0., 10., -15.0), 
        .at = vec_aaa(0.) 
    };

    // Create a new `Scene`
    Scene scene = scene_new(camera);

    //
    // Prepare `Material`s, these will be loaded from `.mtl` files later

    Material* shiny_orange = scene_add_material(&scene, (Material) {
        .color_ambient = vec_abc(1., 0.4, 0.),
        .color_diffuse = vec_abc(1., 0.4, 0.),
        .color_spec = vec_abc(1., 0.4, 0.),
        .luster = 1.,
        .metallicity = 125.
    } );

    Material* blue = scene_add_material(&scene, (Material) {
        .color_ambient = vec_abc(0.2, 0.2, 1.),
        .color_diffuse = vec_abc(0.2, 0.2, 1.),
        .color_spec = vec_abc(0.2, 0.2, 1.),
        .luster = 0.5,
        .metallicity = 50.
    } );

    Material* muddy_green = scene_add_material(&scene, (Material) {
        .color_ambient = vec_abc(0.2, 0.4, 0.),
        .color_diffuse = vec_abc(0.2, 0.4, 0.),
        .color_spec = vec_abc(0.2, 0.4, 0.),
        .luster = 1.,
        .metallicity = 75.
    } );

    //
    // Load the Utah Teapot

    Mesh* teapot = scene_add_mesh(&scene, mesh_from_raw(TEAPOT, shiny_orange), STATIC);
    mesh_transform(teapot, transform_rotate(Z, 1.570796));

    //
    // `Light`s

    scene_add_light(&scene, light_new(vec_abc(15., 10., 0.), 0.8));
    scene_add_light(&scene, light_new(vec_abc(-15., 10., 0.), 0.8));

    //
    // `Sphere`s

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

    // Initialize the scene
    scene_initialize(&scene);

    // Demonstrate that transforming a DYNAMIC object after initialization is allowed
    sphere_transform(dyn_sphere, transform_translate(vec_abc(0., 1., -1.)));
    
    // Create a new `Buffer`
    Buffer b = buffer_wh(640, 360);

    // Write the ray traced image to the `Buffer`
    raytrace(b, scene, config);

    // Export `Buffer` as a PPM image
    buffer_export_as_ppm(b, "test.ppm");

    // Free memory before exit
    scene_free(&scene); buffer_free(&b);

    printf("Complete...\n");

    return 0;
}