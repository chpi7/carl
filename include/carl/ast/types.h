#ifndef CARL_TYPES_H
#define CARL_TYPES_H

#include <vector>
#include <string>
#include <memory>
#include <optional>

namespace carl {
namespace types {

enum class BaseType { UNKNOWN, BOOL, STRING, INT, FLOAT, FN };

class Type {
   public:
    virtual BaseType get_base_type() = 0;
    virtual bool equals(Type* other) { return get_base_type() == other->get_base_type(); }
    virtual bool is_number() { return false; }
    virtual bool can_assign(Type* other) { return get_base_type() == other->get_base_type(); }
    virtual bool can_cast_to(Type* other) { return get_base_type() == other->get_base_type(); }
    virtual std::string str() = 0;
};

class Unknown : public Type {
    BaseType get_base_type() { return BaseType::UNKNOWN; };
    std::string str() {
        return std::string("?");
    }
};

class Number : public Type {
    bool is_number() { return true; }
};

class Int : public Number {
    BaseType get_base_type() { return BaseType::INT; };
    bool can_cast_to(Type* other) {
        if (other->get_base_type() == BaseType::FLOAT) return true;
        return get_base_type() == other->get_base_type();
    }
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
    Fn(std::vector<std::shared_ptr<Type>> parameters, std::optional<std::shared_ptr<Type>> ret)
        : parameters(parameters), ret(ret){};
    BaseType get_base_type() { return BaseType::FN; };

    const std::vector<std::shared_ptr<Type>> get_parameters() {
        return parameters;
    };
    const std::optional<std::shared_ptr<Type>> get_ret() { return ret; };

    std::string str() {
        auto result = std::string("(");
        bool first = true;
        for (auto& param : parameters) {
            if (!first) result += ", ";
            result += param->str();
            first = false;
        }
        result += ") -> " + (ret ? ret.value()->str() : "void");
        return result;
    }
};
}  // namespace types
}  // namespace carl

#endif