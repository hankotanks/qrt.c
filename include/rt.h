#ifndef RT_H
#define RT_H

#include "geom.h"
#include "buffer.h"
#include "in.h"
#include "intrs.h"
#include "scene.h"

// TODO: Remove test function once Rust FFI is stable
int test(void) { return 1; }

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

Vec cast(Scene s, Config c, size_t h, size_t w, size_t x, size_t y) {
    Ray r = camera_ray(s, h, w, x, y);

    Intersection intrs = intersection_check(s, c, r);
    if(!intrs.s.st) return vec_aaa(0.);

    Vec normal, hit;
    intersection_normal(intrs, r, &normal, &hit);

    Material* material = intersection_material(intrs);

    Vec pixel_color = mul_vs(material->color_ambient, c.ambience);
    SLL* curr = s.lights; while(curr) {
        Light light = *(Light*) curr->item;

        Ray light_ray = (Ray) {
            .origin = hit,
            .dir = norm_v(sub_vv(light.pos, hit))
        };
        
        Intersection shadow = intersection_check_excl(s, c, light_ray, intrs.s);
        if(!shadow.s.st) {
            double diffuse = MAX(0., dot_vv(normal, light_ray.dir) * light.strength);

            pixel_color = add_vv(pixel_color, mul_vs(material->color_diffuse, diffuse));

            Vec refl = sub_vv(r.dir, mul_vs(normal, 2. * dot_vv(normal, r.dir)));

            double spec = MAX(0., material->luster * pow(dot_vv(refl, light_ray.dir), material->metallicity));

            pixel_color = add_vv(pixel_color, mul_vs(material->color_spec, spec));
        }

        curr = curr->next;
    }

    return clamp_v(pixel_color, 0., 1.);
}

void raytrace(Buffer b, Scene s, Config c) {
    assert(s.tt &&
        "Error: Scene was never initialized");

    size_t x, y;
    for(x = 0; x < b.w; x++)
        for(y = 0; y < b.h; y++) {
            Vec color = cast(s, c, b.h, b.w, x, y);

            buffer_set_pixel(b, x, y, color);
        }
}

#endif /* RT_H */