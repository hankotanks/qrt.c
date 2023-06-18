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
        "    width: %zu\n"
        "    height: %zu\n}\n",
        b->w, b->h
    );
}

//
// Helper functions

size_t size_t_length(size_t val) {
    size_t i = 0;
    while(val != 0) {
        ++i;
        val /= 10;
    }

    return i;
}

void size_t_write_digits(char* s, size_t* i, size_t val, size_t len) {
    size_t j;
    for(j = len; j > 0; j--) {
        s[*i + j - 1] = (char) (val % 10 + 48);
        val /= 10;
    }

    *i += len;
}

//
// Write Buffer to `ppm` image

void buffer_export_as_ppm(Buffer b, char* file) {
    size_t w_len = size_t_length(b.w);
    size_t h_len = size_t_length(b.h);

    size_t s_len = 9 + w_len + h_len;
    char* s = (char*) malloc(s_len);

    s[0] = 'P';
    s[1] = '6';
    s[2] = (char) 10;

    size_t i = 3;
    size_t_write_digits(s, &i, b.w, w_len);

    s[i] = (char) 32;
    ++i;

    size_t_write_digits(s, &i, b.h, h_len);

    s[i++] = (char) 10;
    s[i++] = '2';
    s[i++] = '5';
    s[i++] = '5';
    s[i++] = (char) 10;


    
    FILE* f;
    if(f = fopen(file, "wb+")) {
        fwrite(s, 1, s_len, f);
        fwrite(b.vs, 1, 3 * b.w * b.h, f);
    }

    free(s);
}
 
#endif /* BUFFER_H */