#include "SFML/Graphics.h"

#include "rt.h"

#define TEAPOT "C:\\Users\\hank_7hp7x6m\\Desktop\\projects\\qrt.c\\models\\uteapot"
#define DEMO

#define W 640
#define H 360

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

    // Create a new `Buffer`
    Buffer b = buffer_wh(W, H, RGBA);

    sfVideoMode mode = { W, H, 32 };
    sfRenderWindow* window;
    sfTexture* tex;
    sfSprite* sprite;
    sfEvent event;

    window = sfRenderWindow_create(mode, "rt.c", sfResize | sfClose, NULL);
    if(!window) return 1;

    tex = sfTexture_create(W, H);
    if(!tex) return 1;

    sprite = sfSprite_create();

    sfSprite_setTexture(sprite, tex, sfTrue);
    sfRenderWindow_setFramerateLimit(window, 30);

    while(sfRenderWindow_isOpen(window)) {
        while(sfRenderWindow_pollEvent(window, &event)) {
            if(event.type == sfEvtClosed) sfRenderWindow_close(window);
        }
        
        raytrace(b, scene, config);

        camera_transform(&scene.camera, transform_translate(mul_vs(scene.camera.at, -1.)));
        camera_transform(&scene.camera, transform_rotate(X, 0.08726646));
        camera_transform(&scene.camera, transform_translate(scene.camera.at));
        
        sfTexture_updateFromPixels(tex, b.vs, W, H, 0, 0);

        sfRenderWindow_clear(window, sfBlack);
        sfRenderWindow_drawSprite(window, sprite, NULL);
        sfRenderWindow_display(window);
    }

    scene_free(&scene); buffer_free(&b);

    return 0;

    // Write the ray traced image to the `Buffer`
    raytrace(b, scene, config);

    // Export `Buffer` as a PPM image
    if(buffer_export_as_ppm(b, "test.ppm")) return 1;

    // Free memory before exit
    scene_free(&scene); buffer_free(&b);

    printf("Complete...\n");

    return 0;
}