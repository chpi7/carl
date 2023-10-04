#pragma once

#include "carl/jit2/runtime_types.h"
#include "carl/ast/types.h"

#define CRT_LLVM_TYPE(name, llvm_ctx) (runtime_type_llvm_get__##name(llvm_ctx))

llvm::Type* runtime_type_llvm_get__crt_string(llvm::LLVMContext &context) {
    const char* llvm_type_name = "crt_string";
    llvm::Type* t = llvm::StructType::getTypeByName(context, llvm_type_name);
    if (t == nullptr) {
        t = llvm::StructType::create(llvm_type_name,
                                     llvm::Type::getInt64Ty(context),
                                     llvm::PointerType::get(context, 0));
    }
    return t;
};

llvm::Type* runtime_type_llvm_get__crt_fn(llvm::LLVMContext &context) {
    const char* llvm_type_name = "crt_fn";
    llvm::Type* t = llvm::StructType::getTypeByName(context, llvm_type_name);
    if (t == nullptr) {
        t = llvm::StructType::create(llvm_type_name,
                                     llvm::PointerType::get(context, 0),
                                     llvm::Type::getInt64PtrTy(context));
    }
    return t;
};

llvm::Type* runtime_type_llvm_get__from_BaseType(carl::types::BaseType base_type, llvm::LLVMContext &context) {
    switch (base_type) {
        case carl::types::BaseType::BOOL:
        case carl::types::BaseType::INT:
            return llvm::Type::getInt64Ty(context);
        case carl::types::BaseType::FLOAT:
            return llvm::Type::getDoubleTy(context);
        case carl::types::BaseType::STRING:
            return runtime_type_llvm_get__crt_string(context);
        default:
            fprintf(stderr, "ERROR in runtime_type_llvm_get__from_BaseType: unmapped type\n");
            return nullptr;
    }
};
