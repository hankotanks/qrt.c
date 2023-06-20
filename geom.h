#ifndef GEOM_H
#define GEOM_H

#include "lalg.h"

#define MIN(i, j) (((i) < (j)) ? (i) : (j))

#define EPS_TI 0.0000001

//
// `Ray` declaration

typedef struct Ray {
    Vec origin;
    Vec dir;
} Ray;

void ray_print_internal(Ray* r, char* name, size_t indent) {
    int id = 4 * (int) indent; 

    if(name) 
        printf("%.*s%s (ray) {\n", id, SP, name);
    else
        printf("%.*s ray {\n", id, SP);

    vec_print_internal(&(r->origin), "origin", indent + 1);
    vec_print_internal(&(r->dir), "dir", indent + 1);

    printf("%.*s}\n", id, SP);
}

void ray_print(Ray* r) {
    ray_print_internal(r, NULL, 0);
}

//
// `Sphere` declaration

typedef struct Sphere {
    Vec center;
    double radius;
} Sphere;

void sphere_print_internal(Sphere* s, char* name, size_t indent) {
    int id = 4 * (int) indent; 

    if(name)
        printf("%.*s%s (sphere) {\n", id, SP, name);
    else 
        printf("%.*s sphere {\n", id, SP);

    vec_print_internal(&(s->center), "center", indent + 1);

    printf(
        "%.*s    radius: %.4lf\n%.*s}\n", 
        id, SP, s->radius, 
        id, SP
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

typedef struct Light {
    Vec pos;
    double strength;
} Light;

void light_print_internal(Light* l, char* name, size_t indent) {
    int id = 4 * (int) indent;

    if(name)
        printf("%.*s%s (light) {\n", id, SP, name);
    else
        printf("%.*s light {\n", id, SP);

    vec_print_internal(&(l->pos), "pos", indent + 1);
    
    printf(
        "%.*s    strength: %lf\n%.*s}\n", 
        id, SP, l->strength,
        id, SP
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
        printf("%.*s%s (vertex) {\n", id, SP, name);
    else
        printf("%.*s vertex {\n", id, SP);

    vec_print_internal(&(v->point), "point", indent + 1);
    vec_print_internal(&(v->normal), "normal", indent + 1);

    printf("%.*s}\n", id, SP);
}

void vertex_print(Vertex* v) {
    vertex_print_internal(v, NULL, 0);
}

//
// `Tri` declaration

typedef struct Tri {
    Vertex* a;
    Vertex* b;
    Vertex* c;
    Vec centroid;
} Tri;

Tri tri_vvv(Vertex* a, Vertex* b, Vertex* c) {
    Vec centroid = (Vec) {
        .x = (a->point.x + b->point.x + c->point.x) / 3.0,
        .y = (a->point.y + b->point.y + c->point.y) / 3.0,
        .z = (a->point.z + b->point.z + c->point.z) / 3.0
    };

    return (Tri) { a, b, c, centroid };
}

void tri_print_internal(Tri* t, char* name, int indent) {
    int id = 4 * (int) indent;

    if(name)
        printf("%.*s%s (tri) {\n", id, SP, name);
    else
        printf("%.*s tri {\n", id, SP);

    vertex_print_internal(t->a, "a", indent + 1);
    vertex_print_internal(t->b, "b", indent + 1);
    vertex_print_internal(t->c, "c", indent + 1);

    vec_print_internal(&(t->centroid), "centroid", indent + 1);

    printf("%.*s}\n", id, SP);
}

void tri_print(Tri* t) {
    tri_print_internal(t, NULL, 0);
}

double tri_intersection(Tri t, Ray r, double t_min, double t_max) {
    Vec e1 = sub_vv(t.b->point, t.a->point);
    Vec e2 = sub_vv(t.c->point, t.a->point);

    Vec p_vec = cross_vv(r.dir, e2);
    Vec t_vec = sub_vv(r.origin, t.a->point);
    Vec q_vec = cross_vv(t_vec, e1);

    double det = dot_vv(e1, p_vec);

    double u, v;
    if(det > EPS_TI) {
        u = dot_vv(t_vec, p_vec);
        if(u < 0. || u > det) return t_max + 1.;

        v = dot_vv(r.dir, q_vec);
        if(v < 0. || u + v > det) return t_max + 1.;
    } else if(det < -1. * EPS_TI) {
        u = dot_vv(t_vec, p_vec);
        if(u > 0. || u < det) return t_max + 1.;

        v = dot_vv(r.dir, q_vec);
        if(v > 0. || u + v < det) return t_max + 1.;
    } else return t_max + 1.;

    double w = dot_vv(e2, q_vec) / det;
    return (w > t_max || w < t_min) ? t_max + 1. : w;
}

int tri_eq(Tri* a, Tri* b) {
    return (a->a == b->a && a->b == b->b && a->c == b->c);
}

//
// `Mesh` declaration

typedef struct Mesh {
    size_t vc;
    size_t tc;
    Vertex* vertices;
    Tri* tris;
} Mesh;

void mesh_free(Mesh* m) {
    free(m->vertices);
    free(m->tris);
}

#endif /* GEOM_H */