#ifndef CARL_TYPES_H
#define CARL_TYPES_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

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
};

class Float : public Number {
    BaseType get_base_type();
    std::string str();
};

class Bool : public Type {
    BaseType get_base_type();
    std::string str();
};

class String : public Type {
    BaseType get_base_type();
    std::string str();
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
    Fn(std::vector<std::shared_ptr<Type>> parameters,
       std::optional<std::shared_ptr<Type>> ret);
    BaseType get_base_type();

    bool can_apply_to(std::vector<std::shared_ptr<Type>> arguments);
    const std::vector<std::shared_ptr<Type>> get_parameters();
    const std::optional<std::shared_ptr<Type>> get_ret();
    std::string str();
};
}  // namespace types
}  // namespace carl

#endif