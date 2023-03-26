#ifndef carl_llvm_runtime_types
#define carl_llvm_runtime_types

#include <cstdint>

namespace carl {
extern "C" {

typedef struct {
    uint64_t len;
    const char* str;
} __carl_string;

typedef struct {
    void* fn_impl;
} __carl_fn;
}
}  // namespace carl

#endif