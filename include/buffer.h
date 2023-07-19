#ifndef BUFFER_H
#define BUFFER_H

#include<stdlib.h>
#include<stdio.h>

//
// `Buffer` declaration

typedef struct Buffer {
    size_t w;
    size_t h;
    char* vs;
} Buffer;

Buffer buffer_wh(size_t w, size_t h) {
    Buffer init = (Buffer) {
        .w = w,
        .h = h,
        .vs = calloc(3 * w * h, sizeof *(init.vs))
    };

    return init;
}

void buffer_free(Buffer* b) {
    free(b->vs);
}

void buffer_print(Buffer* b) {
    printf(
        "buffer {\n"
        "    width: %u\n"
        "    height: %u\n}\n",
        (unsigned) b->w, 
        (unsigned) b->h
    );
}

//
// Helper functions

size_t helper_size_t_length(size_t val) {
    size_t i = 0;
    while(val != 0) {
        ++i;
        val /= 10;
    }

    return i;
}

void helper_size_t_write_digits(char* s, size_t* i, size_t val, size_t len) {
    size_t j;
    for(j = len; j > 0; j--) {
        s[*i + j - 1] = (char) (val % 10 + 48);
        val /= 10;
    }

    *i += len;
}

//
// Set pixel

void buffer_set_pixel(Buffer b, size_t x, size_t y, Vec c) {
    int i = 3 * (x + y * b.w);

    b.vs[i + 0] = (char) (int) (c.x * 255.);
    b.vs[i + 1] = (char) (int) (c.y * 255.);
    b.vs[i + 2] = (char) (int) (c.z * 255.);
}

//
// Write Buffer to `ppm` image

void buffer_export_as_ppm(Buffer b, char* file) {
    size_t w_len = helper_size_t_length(b.w);
    size_t h_len = helper_size_t_length(b.h);

    size_t s_len = 9 + w_len + h_len;
    char* s = (char*) malloc(s_len);

    s[0] = 'P';
    s[1] = '6';
    s[2] = (char) 10;

    size_t i = 3;
    helper_size_t_write_digits(s, &i, b.w, w_len);

    s[i] = (char) 32;
    ++i;

    helper_size_t_write_digits(s, &i, b.h, h_len);

    s[i++] = (char) 10;
    s[i++] = '2';
    s[i++] = '5';
    s[i++] = '5';
    s[i++] = (char) 10;

    FILE* f;
    if((f = fopen(file, "wb+"))) {
        fwrite(s, 1, s_len, f);
        fwrite(b.vs, 1, 3 * b.w * b.h, f);
    }

    fclose(f);
    free(s);
}
 
#endif /* BUFFER_H */