#ifndef carl_llvm_runtime_types
#define carl_llvm_runtime_types

#include <cstdint>

#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"

namespace carl {
extern "C" {

typedef struct {
    uint64_t len;
    uint64_t max_len;
    uint64_t* data;
} __carl_vec;

typedef struct {
    uint64_t len;
    const char* str;
} __carl_string;

typedef struct {
    void* fn_impl;
    __carl_vec* captured_values;
} __carl_fn;
}

llvm::Type* carl_fn_llvm_type(llvm::LLVMContext& ctx);
llvm::Type* carl_vec_llvm_type(llvm::LLVMContext& ctx);
llvm::Type* carl_string_llvm_type(llvm::LLVMContext& ctx);
}  // namespace carl

#endif