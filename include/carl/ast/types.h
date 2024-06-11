#ifndef CARL_TYPES_H
#define CARL_TYPES_H

#include <memory>
#include <string>
#include <vector>

#include "llvm/IR/Type.h"

namespace carl {
namespace types {

enum class BaseType { UNKNOWN, BOOL, STRING, INT, FLOAT, VOID, FN, ADT };

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
    virtual bool is_rt_heap_obj() { return false; }
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
};

class Float : public Number {
    BaseType get_base_type();
    std::string str() const;
};

class Bool : public Type {
    BaseType get_base_type();
    std::string str() const;
};

class String : public Type {
   private:
    std::string llvm_type_name = std::string("__carl_string");

   public:
    BaseType get_base_type();
    std::string str() const;
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
    bool is_rt_heap_obj();
    bool can_cast_to(Type* other);
    bool equals(Type* other);

    bool can_apply_to(std::vector<std::shared_ptr<Type>> arguments);
    const std::vector<std::shared_ptr<Type>> get_parameters();
    const std::shared_ptr<Type> get_ret();
    std::string str() const;
};

class Adt : public Type {
   public:
    struct Constructor {
        std::string name;
        std::vector<std::shared_ptr<Type>> members;
        void add_member(std::shared_ptr<Type>&& member);
    };

   private:
    std::vector<Constructor> constructors;
    std::string name;

   public:
    Adt(std::string name, std::vector<Constructor> constructors)
        : name(name), constructors(constructors){};
    BaseType get_base_type();
    std::string str() const;
};

class RefByName : public Type {
   private:
    std::string name;

   public:
    RefByName(std::string name) : name(name){};
    BaseType get_base_type();
    std::string str() const;
};

}  // namespace types
}  // namespace carl

#endif
