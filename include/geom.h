#ifndef GEOM_H
#define GEOM_H

#include "lalg.h"

#define EPS_TRI 0.0000001

//
// `Ray` declaration

typedef struct Ray {
    Vec origin;
    Vec dir;
} Ray;

void ray_print_internal(Ray* r, char* name, size_t indent) {
    int id = 4 * (int) indent; 

    if(name) 
        printf("%.*s%s (ray) {\n", id, PADDING, name);
    else
        printf("%.*s ray {\n", id, PADDING);

    vec_print_internal(&(r->origin), "origin", indent + 1);
    vec_print_internal(&(r->dir), "dir", indent + 1);

    printf("%.*s}\n", id, PADDING);
}

void ray_print(Ray* r) {
    ray_print_internal(r, NULL, 0);
}

//
// `Material` declaration

typedef struct Material Material;

struct Material {
    char* name;
    Vec color_ambient;
    Vec color_diffuse;
    Vec color_spec;
    double luster;
    double metallicity;
    Material* next;
};

//
// `Sphere` declaration

typedef struct Sphere Sphere;

struct Sphere {
    Vec center;
    double radius;
    Material* material;
    Sphere* next;
};

void sphere_print_internal(Sphere* s, char* name, size_t indent) {
    int id = 4 * (int) indent; 

    if(name)
        printf("%.*s%s (sphere) {\n", id, PADDING, name);
    else 
        printf("%.*s sphere {\n", id, PADDING);

    vec_print_internal(&(s->center), "center", indent + 1);

    printf(
        "%.*s    radius: %.4lf\n%.*s}\n", 
        id, PADDING, s->radius, 
        id, PADDING
    );
}

void sphere_print(Sphere* s) {
    sphere_print_internal(s, NULL, 0);
}

double sphere_intersection(Sphere s, Ray r, double t_min, double t_max) {
    double rad_sq = s.radius * s.radius;

    Vec l = sub_vv(s.center, r.origin);

    double tca = dot_vv(l, norm_v(r.dir));
    double d_sq = dot_vv(l, l) - tca * tca;

    if(d_sq > rad_sq) return t_max + 1.;

    double thc = sqrt(rad_sq - d_sq);

    double t = tca - thc;
    double w = tca + thc;

    double len = len_v(r.dir);
    (t < t_max && t > t_min) ? t = t / len : (t = -1.);
    (w < t_max && w > t_min) ? w = w / len : (w = -1.);

    if(t > 0. && w == -1.) return t;
    if(w > 0. && w == -1.) return w;

    if(t > 0. && w > 0.) return MIN(t, w);

    return t_max + 1.;
}

//
// `Light` declaration

typedef struct Light Light;

struct Light {
    Vec pos;
    double strength;
    Light* next;
};

Light light_new(Vec pos, double strength) {
    return (Light) { .pos = pos, .strength = strength, .next = NULL };
}

void light_print_internal(Light* l, char* name, size_t indent) {
    int id = 4 * (int) indent;

    if(name)
        printf("%.*s%s (light) {\n", id, PADDING, name);
    else
        printf("%.*s light {\n", id, PADDING);

    vec_print_internal(&(l->pos), "pos", indent + 1);
    
    printf(
        "%.*s    strength: %lf\n%.*s}\n", 
        id, PADDING, l->strength,
        id, PADDING
    );
}

void light_print(Light* l) {
    light_print_internal(l, NULL, 0);
}

//
// `Vertex` declaration

typedef struct Vertex {
    Vec point;
    Vec normal;
} Vertex;

void vertex_print_internal(Vertex* v, char* name, size_t indent) {
    int id = 4 * (int) indent;

    if(name)
        printf("%.*s%s (vertex) {\n", id, PADDING, name);
    else
        printf("%.*s vertex {\n", id, PADDING);

    vec_print_internal(&v->point, "point", indent + 1);
    vec_print_internal(&v->normal, "normal", indent + 1);

    printf("%.*s}\n", id, PADDING);
}

void vertex_print(Vertex* v) {
    vertex_print_internal(v, NULL, 0);
}

//
// `Tri` declaration

typedef struct Tri {
    Vertex a;
    Vertex b;
    Vertex c;
    Vec centroid;
    Material* material;
} Tri;

Vec helper_tri_centroid(Vertex a, Vertex b, Vertex c) {
    return (Vec) {
        .x = (a.point.x + b.point.x + c.point.x) / 3.0,
        .y = (a.point.y + b.point.y + c.point.y) / 3.0,
        .z = (a.point.z + b.point.z + c.point.z) / 3.0
    };
}

Tri tri_new(Vertex a, Vertex b, Vertex c, Material* material) {
    Vec centroid = helper_tri_centroid(a, b, c);

    return (Tri) { a, b, c, centroid, material };
}

void tri_print_internal(Tri* t, char* name, int indent) {
    int id = 4 * (int) indent;

    if(name)
        printf("%.*s%s (tri) {\n", id, PADDING, name);
    else
        printf("%.*s tri {\n", id, PADDING);

    vertex_print_internal(&t->a, "a", indent + 1);
    vertex_print_internal(&t->b, "b", indent + 1);
    vertex_print_internal(&t->c, "c", indent + 1);

    vec_print_internal(&t->centroid, "centroid", indent + 1);

    printf("%.*s}\n", id, PADDING);
}

void tri_print(Tri* t) {
    tri_print_internal(t, NULL, 0);
}

double tri_intersection(Tri t, Ray r, double t_min, double t_max) {
    Vec e1 = sub_vv(t.b.point, t.a.point);
    Vec e2 = sub_vv(t.c.point, t.a.point);

    Vec p_vec = cross_vv(r.dir, e2);
    Vec t_vec = sub_vv(r.origin, t.a.point);
    Vec q_vec = cross_vv(t_vec, e1);

    double det = dot_vv(e1, p_vec);

    double u, v;
    if(det > EPS_TRI) {
        u = dot_vv(t_vec, p_vec);
        if(u < 0. || u > det) return t_max + 1.;

        v = dot_vv(r.dir, q_vec);
        if(v < 0. || u + v > det) return t_max + 1.;
    } else if(det < -1. * EPS_TRI) {
        u = dot_vv(t_vec, p_vec);
        if(u > 0. || u < det) return t_max + 1.;

        v = dot_vv(r.dir, q_vec);
        if(v > 0. || u + v < det) return t_max + 1.;
    } else return t_max + 1.;

    double w = dot_vv(e2, q_vec) / det;
    return (w > t_max || w < t_min) ? t_max + 1. : w;
}

//
// `Mesh` declaration

typedef struct Mesh Mesh;

struct Mesh {
    char* name;
    size_t tc;
    Tri* tris;
    Mesh* next;
};

//
// `Axis` declaration

typedef enum Axis { X, Y, Z } Axis;

//
// `Transform` declaration

typedef enum TransformType { ROTATE, SCALE, TRANSLATE } TransformType;

typedef struct Transform {
    TransformType tt;
    Vec a;
    double t;
} Transform;

void transform_print(Transform* t); // TODO

Transform transform_rotate(Axis axis, double angle) {
    Vec temp = (Vec) {
        .x = (double) (axis == X),
        .y = (double) (axis == Y),
        .z = (double) (axis == Z)
    };

    return (Transform) {
        .tt = ROTATE,
        .a = temp,
        .t = angle
    };
}

Transform transform_scale(Vec factor) {
    return (Transform) {
        .tt = SCALE,
        .a = factor
    };
}

Transform transform_translate(Vec offset) {
    return (Transform) {
        .tt = TRANSLATE,
        .a = offset
    };
}

//
// `Mesh` functions

void mesh_transform(Mesh* mesh, Transform t) {
    Mat m;
    switch(t.tt) {
        case ROTATE: {
                Mat mx, my, mz;
                mx = rot_x(t.t * t.a.x);
                my = rot_y(t.t * t.a.y);
                mz = rot_z(t.t * t.a.z);

                Mat temp;
                temp = mul_mm(mx, my); mat_free(&mx);   mat_free(&my);
                m = mul_mm(temp, mz);  mat_free(&temp); mat_free(&mz);
        }; break;
        case SCALE: m = scale(t.a); break;
        case TRANSLATE: m = translate(t.a); break;
    }

    size_t i;
    for(i = 0; i < mesh->tc; i++) {
        Tri* tri = &(mesh->tris[i]);
        tri->a.point = mul_vm(tri->a.point, m, POINT);
        tri->b.point = mul_vm(tri->b.point, m, POINT);
        tri->c.point = mul_vm(tri->c.point, m, POINT);
        tri->centroid = helper_tri_centroid(tri->a, tri->b, tri->c);
    }

    mat_free(&m);
}

#endif /* GEOM_H */