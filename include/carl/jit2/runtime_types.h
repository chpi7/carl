#pragma once

#include <cstdint>

typedef struct crt_string {
    uint64_t len;
    const char* data;
} crt_string;

typedef struct crt_fn {
    void* fn_impl;
    uint64_t* captures;
} crt_fn;
