#include "lalg.h"
#include "geom.h"
#include "in.h"
#include "buffer.h"

//
// `Camera declaration

typedef struct Camera {
    Vec pos;
    Vec at;
} Camera;

//
// `Scene` declaration

typedef struct Scene {
    Mesh* meshes;
    size_t mc;
    Sphere* spheres;
    size_t sc;
    Light* lights;
    size_t lc;
    Camera camera;
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
// Main function

int main(void) {
    Mesh m = mesh_from_raw_vvvnnn("./models/uteapot_vvvnnn");

    mesh_print(&m);

    tri_print(&(m.tris[4]));

    Buffer b = buffer_wh(640, 360);
    buffer_export_as_ppm(b, "./test.ppm");
    buffer_free(&b);
    mesh_free(&m);

    return 0;
}