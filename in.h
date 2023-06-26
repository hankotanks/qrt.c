#ifndef IN_H
#define IN_H

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include "geom.h"

void size_t_read(FILE* f, size_t* val) {
    unsigned int i;
    fscanf(f, "%u\n", &i);

    *val = (size_t) i;
}

Mesh* mesh_from_raw_vvvnnn(char* name, char* file, Material* material) {
    FILE* f = fopen(file, "r");

    size_t tc, i;
    size_t_read(f, &tc);

    Mesh* m = malloc(sizeof *m);
    *m = (Mesh) {
        .name = malloc(strlen(name) + 1),
        .tc = tc,
        .tris = calloc(tc, sizeof *m->tris),
        .next = NULL
    }; strcpy(m->name, name);

    Vertex a, b, c;
    for(i = 0; i < tc; i++) {
        fscanf(f, 
            "%lf %lf %lf\n"
            "%lf %lf %lf\n"
            "%lf %lf %lf\n"
            "%lf %lf %lf\n"
            "%lf %lf %lf\n"
            "%lf %lf %lf\n\n",
            &a.point.x,  &a.point.y,  &a.point.z,
            &a.normal.x, &a.normal.y, &a.normal.z,
            &b.point.x,  &b.point.y,  &b.point.z,
            &b.normal.x, &b.normal.y, &b.normal.z,
            &c.point.x,  &c.point.y,  &c.point.z,
            &c.normal.x, &c.normal.y, &c.normal.z
        );

        m->tris[i] = tri_vvv(a, b, c, material);
    }

    fclose(f);

    return m;
}

Mesh* mesh_from_obj(char* file); // TODO

#endif /* IN_H */