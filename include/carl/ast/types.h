#ifndef CARL_TYPES_H
#define CARL_TYPES_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"

namespace carl {
namespace types {

enum class BaseType { UNKNOWN, BOOL, STRING, INT, FLOAT, VOID, FN };

class Type {
   public:
    virtual BaseType get_base_type() = 0;
    virtual bool equals(Type* other) {
        return get_base_type() == other->get_base_type();
    }
    virtual bool is_number() { return false; }
    virtual bool can_assign(Type* other) {
        return get_base_type() == other->get_base_type();
    }
    virtual bool can_cast_to(Type* other) {
        return get_base_type() == other->get_base_type();
    }
    virtual std::string str() = 0;
    virtual llvm::Type* get_llvm_type(llvm::LLVMContext& ctx) {
        return llvm::Type::getVoidTy(ctx);
    }
};

class Void : public Type {
    BaseType get_base_type();
    std::string str();
};

class Unknown : public Type {
    BaseType get_base_type();
    std::string str();
};

class Number : public Type {
    bool is_number();
};

class Int : public Number {
    BaseType get_base_type();
    bool can_cast_to(Type* other);
    std::string str();
    llvm::Type* get_llvm_type(llvm::LLVMContext& ctx);
};

class Float : public Number {
    BaseType get_base_type();
    std::string str();
    llvm::Type* get_llvm_type(llvm::LLVMContext& ctx);
};

class Bool : public Type {
    BaseType get_base_type();
    std::string str();
    llvm::Type* get_llvm_type(llvm::LLVMContext& ctx);
};

/**
 *  Allocated as:
 *  {
 *      int len; // incl. null
 *      char* text; // null terminated, on the heap
 *  }
*/
class String : public Type {
    private:
    std::string llvm_type_name = std::string("__carl_string");
    public:
    BaseType get_base_type();
    std::string str();
    llvm::Type* get_llvm_type(llvm::LLVMContext& ctx);
};

class Fn : public Type {
   private:
    std::vector<std::shared_ptr<Type>> parameters;
    std::shared_ptr<Type> ret;

   public:
    Fn();
    Fn(std::vector<std::shared_ptr<Type>> parameters);
    Fn(std::vector<std::shared_ptr<Type>> parameters,
       std::shared_ptr<Type> ret);
    BaseType get_base_type();
    llvm::Type* get_llvm_type(llvm::LLVMContext& ctx);

    bool can_apply_to(std::vector<std::shared_ptr<Type>> arguments);
    const std::vector<std::shared_ptr<Type>> get_parameters();
    const std::shared_ptr<Type> get_ret();
    std::string str();
};
}  // namespace types
}  // namespace carl

#endif