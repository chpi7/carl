#include "carl/llvm/runtime_types.h"

namespace carl {

llvm::Type* carl_fn_llvm_type(llvm::LLVMContext& ctx) {
    llvm::Type* t = llvm::StructType::getTypeByName(ctx, "__carl_fn");
    auto* ptr_t = llvm::PointerType::get(ctx, 0);
    if (t == nullptr) {
        t = llvm::StructType::create("__carl_fn", ptr_t, ptr_t);
    }
    return t;
}

llvm::Type* carl_vec_llvm_type(llvm::LLVMContext& ctx) {
    llvm::Type* t = llvm::StructType::getTypeByName(ctx, "__carl_vec");
    auto* ptr_t = llvm::PointerType::get(ctx, 0);
    auto* int_t = llvm::IntegerType::getInt64Ty(ctx);
    if (t == nullptr) {
        t = llvm::StructType::create("__carl_vec", int_t, int_t, ptr_t);
    }
    return t;
}

llvm::Type* carl_string_llvm_type(llvm::LLVMContext& ctx) {
    llvm::Type* t = llvm::StructType::getTypeByName(ctx, "__carl_string");
    auto* ptr_t = llvm::PointerType::get(ctx, 0);
    auto* int_t = llvm::IntegerType::getInt64Ty(ctx);
    if (t == nullptr) {
        t = llvm::StructType::create("__carl_string", int_t, ptr_t);
    }
    return t;
}
}