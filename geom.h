#ifndef GEOM_H
#define GEOM_H

#include "lalg.h"

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

//
// `Mesh` declaration

typedef struct Mesh {
    Vertex* vertices;
    size_t vc;
    Tri* tris;
    size_t tc;
} Mesh;

void mesh_free(Mesh* m) {
    free(m->vertices);
    free(m->tris);
}

void mesh_print(Mesh* m) {
    printf(
        "mesh {\n    tc: %u\n    vc: %u\n}\n", 
        (unsigned) m->tc, 
        (unsigned) m->vc
    );
}


#endif /* GEOM_H */