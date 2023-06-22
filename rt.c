#include "lalg.h"
#include "geom.h"
#include "in.h"
#include "buffer.h"
#include "scene.h"

//
// Raytracing

Ray camera_ray(Scene s, size_t h, size_t w, size_t x, size_t y) {
    Vec camera_dir = norm_v(sub_vv(s.camera.at, s.camera.pos));

    Vec up = vec_abc(0., -1., 0.);
    Vec right = cross_vv(camera_dir, up);

    double norm_x = ((double) x / (double) w) - 0.5;
    double norm_y = ((double) y / (double) h) - 0.5;

    Vec i = mul_vs(right, norm_x);
    Vec j = mul_vs(up, norm_y);

    Vec img_pt = add_vv(add_vv(add_vv(i, j), s.camera.pos), camera_dir);

    return (Ray) {
        .origin = s.camera.pos,
        .dir = norm_v(sub_vv(img_pt, s.camera.pos))
    };
}

Vec helper_tri_normal(Tri t, Vec pos) {
    Vec a, b, c;
    a = t.a->point;
    b = t.b->point;
    c = t.c->point;
    
    Vec v0, v1, v2;
    v0 = sub_vv(b, a);
    v1 = sub_vv(c, a);
    v2 = sub_vv(pos, a);

    double d00, d01, d11, d20, d21;
    d00 = dot_vv(v0, v0);
    d01 = dot_vv(v0, v1);
    d11 = dot_vv(v1, v1);
    d20 = dot_vv(v2, v0);
    d21 = dot_vv(v2, v1);
    
    double denom = d00 * d11 - d01 * d01;

    double v, w, u;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1. - v - w;

    Vec na, nb, nc;
    na = mul_vs(t.a->normal, v);
    nb = mul_vs(t.b->normal, w);
    nc = mul_vs(t.c->normal, u);

    return add_vv(add_vv(na, nb), nc);
}

void intersection_normal(Intersection i, Ray r, Vec* normal, Vec* hit) {
    *hit = add_vv(r.origin, mul_vs(r.dir, i.t));

    switch(i.s) {
        case SPHERE: 
            *normal = norm_v(sub_vv(*hit, i.sphere.center));
            break;
        case TRI: 
            *normal = helper_tri_normal(i.tri, *hit);
            break;
        default: break;
    }
}

Vec cast(Scene s, Config c, size_t h, size_t w, size_t x, size_t y) {
    Ray r = camera_ray(s, h, w, x, y);

    Intersection intrs = intersection_check(s, c, r);
    if(intrs.s == NONE) return vec_aaa(0.);

    Vec normal, hit;
    intersection_normal(intrs, r, &normal, &hit);

    double light = c.ambience;
    
    size_t i;
    for(i = 0; i < s.lc && intrs.s != NONE; i++) {
        Ray light_ray = (Ray) {
            .origin = hit,
            .dir = norm_v(sub_vv(s.lights[i].pos, hit))
        };
        
        Intersection shadow = intersection_check_excl(s, c, light_ray, intersection_into_exclusion(intrs));
        if(shadow.s == NONE) {
            double temp = dot_vv(normal, light_ray.dir) * s.lights[i].strength;
            if(temp >= 0.) {
                Vec refl = sub_vv(r.dir, mul_vs(normal, 2. * dot_vv(normal, r.dir)));

                double spec = dot_vv(refl, light_ray.dir);
                light += MAX(0., 0.9 * pow(spec, 100.));
                light += temp;
            }
        }
    }

    return vec_aaa(MAX(0.0, MIN(1., light)));
}

void raytrace(Buffer b, Scene s, Config c) {
    size_t x, y;
    for(x = 0; x < b.w; x++)
        for(y = 0; y < b.h; y++) {
            Vec color = cast(s, c, b.h, b.w, x, y);

            buffer_set_pixel(b, x, y, color);
        }
}

//
// Main function

int main(void) {
    Mesh m = mesh_from_raw_vvvnnn("./models/uteapot_vvvnnn");

    BVH* h = bvh(1, &m);

    Light l = (Light) { .pos = vec_abc(0., 10., -10.), .strength = 1. };

    Scene s = (Scene) {
        .camera = (Camera) { .pos = vec_abc(0., 5., -10.0), .at = vec_aaa(0.) },
        .mc = 1,
        .sc = 0,
        .lc = 1,
        .meshes = &m,
        .hierarchy = h,
        .spheres = NULL,
        .lights = &l
    };

    Config c = (Config) {
        .t_min = 0.01,
        .t_max = 1000.,
        .fov = 1.570796,
        .ambience = 0.2
    };

    Buffer b = buffer_wh(640, 360);

    raytrace(b, s, c);

    buffer_export_as_ppm(b, "test2.ppm");

    printf("Complete...\n");

    return 0;
}