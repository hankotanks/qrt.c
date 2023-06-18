#include "lalg.h"
#include "geom.h"

int main(void) {
    Vertex a = (Vertex) { .point = vec_aaa(0.f), .normal = vec_aaa(0.5) };
    Vertex b = (Vertex) { .point = vec_aaa(1.f), .normal = vec_aaa(1.5) };
    Vertex c = (Vertex) { .point = vec_aaa(2.f), .normal = vec_aaa(2.5) };
    Vec centroid = vec_abc(0.1, 0.2, 0.3);

    Tri t = (Tri) { &a, &b, &c, centroid };

    tri_print(&t);

    return 0;
}