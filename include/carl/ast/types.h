#ifndef CARL_TYPES_H
#define CARL_TYPES_H

#include <memory>
#include <optional>
#include <string>
#include <vector>
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
    virtual std::string str() const = 0;
    virtual llvm::Type* get_llvm_rt_type(llvm::LLVMContext& ctx) {
        return llvm::Type::getVoidTy(ctx);
    }
    virtual bool is_rt_heap_obj() { return false; }

//    public:
//     friend std::ostream& operator<<(std::ostream& os, const Type& t) {
//         os << t.str();
//         return os;
//     }
};

class Void : public Type {
    BaseType get_base_type();
    std::string str() const;
};

class Unknown : public Type {
    BaseType get_base_type();
    std::string str() const;
};

class Number : public Type {
    bool is_number();
};

class Int : public Number {
    BaseType get_base_type();
    bool can_cast_to(Type* other);
    std::string str() const;
    llvm::Type* get_llvm_rt_type(llvm::LLVMContext& ctx);
};

class Float : public Number {
    BaseType get_base_type();
    std::string str() const;
    llvm::Type* get_llvm_rt_type(llvm::LLVMContext& ctx);
};

class Bool : public Type {
    BaseType get_base_type();
    std::string str() const;
    llvm::Type* get_llvm_rt_type(llvm::LLVMContext& ctx);
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
    std::string str() const;
    llvm::Type* get_llvm_rt_type(llvm::LLVMContext& ctx);
    bool is_rt_heap_obj();
};

class Fn : public Type {
   private:
    std::string llvm_type_name = std::string("__carl_fn");
    std::vector<std::shared_ptr<Type>> parameters;
    std::shared_ptr<Type> ret;

   public:
    Fn();
    Fn(std::vector<std::shared_ptr<Type>> parameters);
    Fn(std::vector<std::shared_ptr<Type>> parameters,
       std::shared_ptr<Type> ret);
    BaseType get_base_type();
    llvm::Type* get_llvm_rt_type(llvm::LLVMContext& ctx);
    llvm::FunctionType* get_llvm_fn_type(llvm::LLVMContext& ctx);
    bool is_rt_heap_obj();

    bool can_apply_to(std::vector<std::shared_ptr<Type>> arguments);
    const std::vector<std::shared_ptr<Type>> get_parameters();
    const std::shared_ptr<Type> get_ret();
    std::string str() const;
};
}  // namespace types
}  // namespace carl

#endif