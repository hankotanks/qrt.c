#ifndef RT_H
#define RT_H

#include<omp.h>

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

// 
// Single-thraded `raytrace` function

void helper_raytrace_standard(Buffer b, Scene s, Config c) {
    size_t x, y;
    for(x = 0; x < b.w; x++)
        for(y = 0; y < b.h; y++) {
            Vec color = cast(s, c, b.h, b.w, x, y);

            buffer_set_pixel(b, x, y, color);
        }
}

//
// `Block` declaration

typedef struct Block {
    int final;
    size_t x_start;
    size_t x_end;
    size_t y_start;
    size_t y_end;
} Block;

// Used to dispatch the next `Block` to a waiting thread
Block next_block(size_t* index, size_t block_w, size_t block_h, size_t block_size) {
    if(*index >= block_w * block_h)
        return (Block) { .final = 1 };

    size_t x_start = (*index) % block_w * block_size;

    size_t y_start = (*index)++ / block_w * block_size;

    return (Block) {
        .x_start = x_start,
        .x_end = x_start + block_size,
        .y_start = y_start,
        .y_end = y_start + block_size
    };
}

//
// Multi-thraded `raytrace` function and combined implementation below

void helper_raytrace_omp(Buffer b, Scene s, Config c) {
    assert((b.w % c.block_size == 0 && b.h % c.block_size == 0) &&
        "Error: Image dimensions must be cleanly divisible by block size");
    
    size_t block_size = c.block_size;
    size_t block_w = b.w / block_size;
    size_t block_h = b.h / block_size;

    omp_set_dynamic(0);
    omp_set_num_threads(c.threads);

    omp_lock_t lock;
    omp_init_lock(&lock);

    size_t i = 0;
    #pragma omp parallel
    {
        omp_set_lock(&lock);
        Block curr = next_block(&i, block_w, block_h, block_size);
        omp_unset_lock(&lock);

        size_t x, y;
        while(!curr.final) {
            for(x = curr.x_start; x < curr.x_end; x++) {
                for(y = curr.y_start; y < curr.y_end; y++) {
                    Vec color = cast(s, c, b.h, b.w, x, y);

                    buffer_set_pixel(b, x, y, color);
                }
            }

            omp_set_lock(&lock);
            curr = next_block(&i, block_w, block_h, block_size);
            omp_unset_lock(&lock);
        }
    }
}

// Combined `raytrace` function
void raytrace(Buffer b, Scene s, Config c) {
    assert(s.tt && "Error: Scene was not initialized");

    if(c.threads == 1)
        helper_raytrace_standard(b, s, c);
    else 
        helper_raytrace_omp(b, s, c);

}

#endif /* RT_H */