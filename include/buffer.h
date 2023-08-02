#ifndef BUFFER_H
#define BUFFER_H

#include<stdlib.h>
#include<stdio.h>
#include<assert.h>

//
// `BufferType` declaration

typedef enum BufferType { RGB, RGBA } BufferType;

//
// `Buffer` declaration

typedef struct Buffer {
    size_t w;
    size_t h;
    BufferType bt;
    unsigned char* vs;
} Buffer;

// Helper function to abstract away the channel count calculation
// (Useful if more `BufferType`s are added later on)
size_t helper_channel_count(BufferType bt) {
    return bt == RGB ? 3 : 4;
}

Buffer buffer_wh(size_t w, size_t h, BufferType bt) {
    Buffer init = (Buffer) {
        .w = w,
        .h = h,
        .bt = bt,
        .vs = calloc(helper_channel_count(bt) * w * h, sizeof *(init.vs))
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

void helper_size_t_write_digits(unsigned char* s, size_t* i, size_t val, size_t len) {
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
    size_t i = helper_channel_count(b.bt)  * (x + y * b.w);

    switch(b.bt) {
        case RGB: {
            b.vs[i + 0] = (char) (int) (c.x * 255.);
            b.vs[i + 1] = (char) (int) (c.y * 255.);
            b.vs[i + 2] = (char) (int) (c.z * 255.);
        } break;
        case RGBA: {
            b.vs[i + 0] = (char) (int) (c.x * 255.);
            b.vs[i + 1] = (char) (int) (c.y * 255.);
            b.vs[i + 2] = (char) (int) (c.z * 255.);
            b.vs[i + 3] = (char) 255;
        } break;
        default: assert(0 && "Error: Unhandled BufferType (cannot set pixel)");
    }
}

//
// Write Buffer to `ppm` image

int buffer_export_as_ppm(Buffer b, char* file) {
    if(b.bt != RGB) 
        assert(0 && "Error: PPM export requires BufferType `RGB`");

    size_t w_len = helper_size_t_length(b.w);
    size_t h_len = helper_size_t_length(b.h);

    size_t s_len = 9 + w_len + h_len;
    unsigned char* s = (unsigned char*) malloc(s_len);

    s[0] = 'P';
    s[1] = '6';
    s[2] = (unsigned char) 10;

    size_t i = 3;
    helper_size_t_write_digits(s, &i, b.w, w_len);

    s[i] = (unsigned char) 32;
    ++i;

    helper_size_t_write_digits(s, &i, b.h, h_len);

    s[i++] = (unsigned char) 10;
    s[i++] = '2';
    s[i++] = '5';
    s[i++] = '5';
    s[i++] = (unsigned char) 10;

    FILE* f;
    if((f = fopen(file, "wb+"))) {
        if(fwrite(s, 1, s_len, f) < s_len) return 1;

        size_t vs_size = 3 * b.w * b.h;
        if(fwrite(b.vs, 1, vs_size, f) < vs_size) return 1;
    }

    fclose(f);
    free(s);

    return 0;
}
 
#endif /* BUFFER_H */