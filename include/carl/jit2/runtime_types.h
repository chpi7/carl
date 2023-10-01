#pragma once

#include <cstdint>

typedef struct crt_string {
    uint64_t len;
    const char* data;
} crt_string;