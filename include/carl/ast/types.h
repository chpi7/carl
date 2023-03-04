#ifndef CARL_TYPES_H
#define CARL_TYPES_H

#include <vector>

namespace carl {
namespace types {

enum class BaseType { BOOL, STRING, INT, FLOAT, FN };

class Type {
    virtual BaseType get_base_type() = 0;
};

class Number : public Type {};

class Int : public Number {
    BaseType get_base_type() { return BaseType::INT; };
};

class Float : public Number {
    BaseType get_base_type() { return BaseType::FLOAT; };
};

class Bool : public Type {
    BaseType get_base_type() { return BaseType::BOOL; };
};

class String : public Type {
    BaseType get_base_type() { return BaseType::STRING; };
};

class Fn : public Type {
   public:
    BaseType get_base_type() { return BaseType::FN; };
};
}  // namespace types
}  // namespace carl

#endif