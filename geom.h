#ifndef GEOM_H
#define GEOM_H

#include "lalg.h"

#define MIN(i, j) (((i) < (j)) ? (i) : (j))

#define EPS 0.0000001

//
// `Ray` declaration

typedef struct Ray {
    Vec origin;
    Vec dir;
} Ray;

void ray_print(Ray* r) {
    printf(
        "ray {\n"
        "    origin { %.4lf, %.4lf, %.4lf }\n"
        "    dir { %.4lf, %.4lf, %.4lf }\n}\n",
        r->origin.x, r->origin.y, r->origin.z,
        r->dir.x, r->dir.y, r->dir.z
    );
}

//
// `Sphere` declaration

typedef struct Sphere {
    Vec center;
    double radius;
} Sphere;

void sphere_print(Sphere* s) {
    printf(
        "sphere {\n"
        "    center { %.4lf, %.4lf, %.4lf }\n"
        "    radius: %.4lf\n}\n",
        s->center.x, s->center.y, s->center.z,
        s->radius
    );
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

//
// `Vertex` declaration

typedef struct Vertex {
    Vec point;
    Vec normal;
} Vertex;

void vertex_print_internal(Vertex* v, char* name, size_t indent) {
    char* sp = "                                ";
    
    int id = 4 * (int) indent;

    printf(
        "%.*s%s {\n"
        "%.*s    point { %.4lf, %.4lf, %.4lf }\n"
        "%.*s    normal { %.4lf, %.4lf, %.4lf }\n%.*s}\n",
        id, sp, name,
        id, sp, v->point.x, v->point.y, v->point.z,
        id, sp, v->normal.x, v->normal.y, v->normal.z,
        id, sp
    );
}

void vertex_print(Vertex* v) {
    vertex_print_internal(v, "vertex", 0);
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

void tri_print(Tri* t) {
    printf("tri {\n");

    vertex_print_internal(t->a, "a", 1);
    vertex_print_internal(t->b, "b", 1);
    vertex_print_internal(t->c, "c", 1);

    printf(
        "    centroid { %.4lf, %.4lf, %.4lf }\n}\n",
        t->centroid.x, t->centroid.y, t->centroid.z
    );
}

double tri_intersection(Tri t, Ray r, double t_min, double t_max) {
    Vec e1 = sub_vv(t.b->point, t.a->point);
    Vec e2 = sub_vv(t.c->point, t.a->point);

    Vec p_vec = cross_vv(r.dir, e2);
    Vec t_vec = sub_vv(r.origin, t.a->point);
    Vec q_vec = cross_vv(t_vec, e1);

    double det = dot_vv(e1, p_vec);

    double u, v;
    if(det > EPS) {
        u = dot_vv(t_vec, p_vec);
        if(u < 0. || u > det) return t_max + 1.;

        v = dot_vv(r.dir, q_vec);
        if(v < 0. || u + v > det) return t_max + 1.;
    } else if(det < -1. * EPS) {
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

void mesh_print(Mesh* m) {
    printf(
        "mesh {\n    vc: %u\n    tc: %u\n}\n", 
        (unsigned) m->vc, 
        (unsigned) m->tc
    );
}

#endif /* GEOM_H */