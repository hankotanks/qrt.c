#ifndef LALG_H
#define LALG_H

#include<stdlib.h>
#include<stdio.h>
#include<math.h>

//
// `Vec` declaration

typedef struct Vec {
	double x;
	double y;
	double z;
} Vec;

Vec vec_abc(double x, double y, double z) {
    return (Vec) { x, y, z };
}

Vec vec_aaa(double a) {
    return (Vec) { a, a, a };
}

void vec_print(Vec* v) {
    printf("vec { %.4lf, %.4lf, %.4lf }\n", v->x, v->y, v->z);
}

typedef enum VecType { VECTOR = 0, POINT = 1 } VecType;

//
// `Mat` declaration

typedef struct Mat { double* vs; } Mat;

Mat mat_id(void) {
    Mat init = (Mat) { calloc(16, sizeof *(init.vs)) };
    
    int i;
    for(i = 0; i < 4; i++) 
        init.vs[i * 5] = 1.f;

    return init;
}

void mat_free(Mat* m) {
    free(m->vs);
}

void mat_print(Mat* m) {
    printf(
        "mat {\n"
        "    %.4lf, %.4lf, %.4lf, %.4lf,\n"
        "    %.4lf, %.4lf, %.4lf, %.4lf,\n"
        "    %.4lf, %.4lf, %.4lf, %.4lf,\n"
        "    %.4lf, %.4lf, %.4lf, %.4lf\n}\n",
        m->vs[0],  m->vs[1],  m->vs[2],  m->vs[3], 
        m->vs[4],  m->vs[5],  m->vs[6],  m->vs[7], 
        m->vs[8],  m->vs[9],  m->vs[10], m->vs[11], 
        m->vs[12], m->vs[13], m->vs[14], m->vs[15] 
    );
}

//
// Linear algebra functions

Vec add_vv(Vec a, Vec b) {
    return (Vec) { a.x + b.x, a.y + b.y, a.z + b.z };
}

Vec sub_vv(Vec a, Vec b) {
    return (Vec) { a.x - b.x, a.y - b.y, a.z - b.z };
}

Vec mul_vs(Vec v, double s) {
    return (Vec) { v.x * s, v.y * s, v.z * s };
}

Vec div_vs(Vec v, double s) {
    return (Vec) { v.x / s, v.y / s, v.z / s };
}

Vec mul_vm(Vec v, Mat m, VecType vt) {
    return (Vec) {
        m.vs[0]  * v.x + m.vs[1]  * v.y + m.vs[2]  * v.z + m.vs[3]  * (double) vt,
        m.vs[4]  * v.x + m.vs[5]  * v.y + m.vs[6]  * v.z + m.vs[7]  * (double) vt,
        m.vs[8]  * v.x + m.vs[9]  * v.y + m.vs[10] * v.z + m.vs[11] * (double) vt
    };
}

Mat mul_mm(Mat m, Mat n) {
    Mat res = (Mat) { calloc(16, sizeof *(res.vs)) };

    size_t x, y, i, k;
    for(x = 0; x < 4; x++) 
        for(y = 0; y < 4; y++) {
            i = x + y * 4;
            for(k = 0; k < 4; k++)
                res.vs[i] += m.vs[x + k * 4] * n.vs[k + y * 4];
        }

    return res;
}

double dot_vv(Vec a, Vec b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec cross_vv(Vec a, Vec b) {
    return (Vec) { 
        a.y * b.z - a.z * b.y, 
        a.z * b.x - a.x * b.z, 
        a.x * b.y - a.y * b.x 
    };
}

double len_v(Vec v) {
    return sqrt(dot_vv(v, v));
}

Vec norm_v(Vec v) {
    return div_vs(v, len_v(v));
}

double distsq_vv(Vec a, Vec b) {
    return pow(a.x - b.x, 2.f) + pow(a.y - b.y, 2.f) + pow(a.z - b.z, 2.f);
}

double dist_vv(Vec a, Vec b) {
    return sqrt(distsq_vv(a, b));
}

//
// Transformations

Mat translate(double x, double y, double z) {
    Mat init = mat_id();

    init.vs[3]  = x;
    init.vs[7]  = y;
    init.vs[11] = z;

    return init;
}

Mat scale(double x, double y, double z) {
    Mat init = (Mat) { calloc(16, sizeof *(init.vs)) };

    init.vs[0]  = x;
    init.vs[5]  = y;
    init.vs[10] = z;
    init.vs[15] = 1.f;

    return init;
}

Mat rot_x(double t) {
    Mat init = mat_id();

    double c = cos(t);
    double s = sin(t);

    init.vs[5]  = c;
    init.vs[6]  = s;
    init.vs[9]  = -1.f * s;
    init.vs[10] = c;

    return init;
}

Mat rot_y(double t) {
    Mat init = mat_id();

    double c = cos(t);
    double s = sin(t);

    init.vs[0]  = c;
    init.vs[2]  = -1.f * s;
    init.vs[8]  = s;
    init.vs[10] = c;

    return init;
}

Mat rot_z(double t) {
    Mat init = mat_id();

    double c = cos(t);
    double s = sin(t);

    init.vs[0] = c;
    init.vs[1] = s;
    init.vs[4] = -1.f * s;
    init.vs[5] = c;

    return init;
}

#endif /* LALG_H */