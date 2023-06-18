#include "lalg.h"

int main(void) {
    Mat m = (Mat) { malloc(16 * (sizeof *(m.vs))) };
    m.vs[0] = 0.f; m.vs[1] = 1.f; m.vs[2] = 2.f; m.vs[3] = 3.f; 
    m.vs[4] = 4.f; m.vs[5] = 5.f; m.vs[6] = 6.f; m.vs[7] = 7.f; 
    m.vs[8] = 8.f; m.vs[9] = 9.f; m.vs[10] = 10.f; m.vs[11] = 11.f; 
    m.vs[12] = 12.f; m.vs[13] = 13.f; m.vs[14] = 14.f; m.vs[15] = 15.f; 

    Mat n = (Mat) { malloc(16 * (sizeof *(n.vs))) };
    n.vs[0] = 0.f; n.vs[1] = 1.f; n.vs[2] = 2.f; n.vs[3] = 3.f; 
    n.vs[4] = 4.f; n.vs[5] = 5.f; n.vs[6] = 6.f; n.vs[7] = 7.f; 
    n.vs[8] = 8.f; n.vs[9] = 9.f; n.vs[10] = 10.f; n.vs[11] = 11.f; 
    n.vs[12] = 12.f; n.vs[13] = 13.f; n.vs[14] = 14.f; n.vs[15] = 15.f; 

    Mat k = mul_mm(m, n);

    //mat_print(k);

    Vec v = vec_abc(0.5, 0.7, 0.8);

    //vec_print(mul_vm(v, k, VECTOR));

    Mat rx = rot_x(0.2617994);
    mat_print(rx);

    return 0;
}