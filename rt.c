#include "lalg.h"
#include "geom.h"
#include "in.h"
#include "buffer.h"
#include "scene.h"

//
// Raytracing

void raytrace(Buffer* b, Scene s, Config c) {

}

//
// Main function

int main(void) {
    Mesh m = mesh_from_raw_vvvnnn("./models/uteapot_vvvnnn");

    Scene s = (Scene) {
        .camera = (Camera) { .pos = vec_aaa(0.), .at = vec_abc(0., 0., 1.) },
        .mc = 1,
        .sc = 0,
        .lc = 0,
        .meshes = &m,
        .spheres = NULL,
        .lights = NULL
    };

    Ray r = (Ray) {
        .origin = vec_abc(0., 0., -10.),
        .dir = vec_abc(0., 0., 1.)
    };

    Config c = (Config) {
        .t_min = 1.,
        .t_max = 1000.,
        .fov = 1.570796,
        .ambience = 0.2
    };

    Intersection i = intersection_check(s, c, r);

    intersection_print(&i);

    return 0;
}