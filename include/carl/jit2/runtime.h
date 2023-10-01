#pragma once

#include <stdlib.h>
#include <string.h>

#include "carl/jit2/runtime_types.h"

typedef std::size_t size_t;

#define CRT_ALLOC(struct_name) ((struct_name*)crt_malloc(sizeof(struct_name)))

extern "C" {

void* crt_malloc(size_t size) {
    void* memory = malloc(size);
    memset(memory, 0, size);
    return memory;
}

crt_string* crt_string__concat(crt_string* a, crt_string* b) {
    crt_string* result = CRT_ALLOC(crt_string);
    result->data = (const char*)crt_malloc(a->len + b->len - 1);
    result->len = a->len + b->len - 1;
    memcpy((void*)result->data, a->data, a->len);
    memcpy((void*)(result->data + a->len - 1), b->data, b->len);
    return result;
}
}