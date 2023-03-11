#ifndef CARL_TYPES_H
#define CARL_TYPES_H

#include <vector>
#include <string>
#include <optional>

namespace carl {
namespace types {

enum class BaseType { UNKNOWN, BOOL, STRING, INT, FLOAT, FN };

class Type {
   public:
    virtual BaseType get_base_type() = 0;
    virtual std::string str() = 0;
};

class Unknown : public Type {
    BaseType get_base_type() { return BaseType::UNKNOWN; };
    std::string str() {
        return std::string("?");
    }
};

class Number : public Type {};

class Int : public Number {
    BaseType get_base_type() { return BaseType::INT; };
    std::string str() {
        return std::string("int");
    }
};

class Float : public Number {
    BaseType get_base_type() { return BaseType::FLOAT; };
    std::string str() {
        return std::string("float");
    }
};

class Bool : public Type {
    BaseType get_base_type() { return BaseType::BOOL; };
    std::string str() {
        return std::string("bool");
    }
};

class String : public Type {
    BaseType get_base_type() { return BaseType::STRING; };
    std::string str() {
        return std::string("string");
    }
};

class Fn : public Type {
   private:
    std::vector<std::shared_ptr<Type>> parameters;
    std::optional<std::shared_ptr<Type>> ret;

   public:
    Fn() : parameters({}), ret({}){};
    Fn(std::vector<std::shared_ptr<Type>> parameters)
        : parameters(parameters), ret({}){};
    Fn(std::vector<std::shared_ptr<Type>> parameters, std::shared_ptr<Type> ret)
        : parameters(parameters), ret(ret){};
    BaseType get_base_type() { return BaseType::FN; };
    std::string str() {
        return std::string("fn(TODO)");
    }
};
}  // namespace types
}  // namespace carl

#endif