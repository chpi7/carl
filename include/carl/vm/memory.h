#ifndef carl_memory_h
#define carl_memory_h

#include <cstdint>

namespace carl {

typedef int64_t carl_int_t;

enum ValueType {
    Nil,
    Bool,
    Int,
    Function,
    Closure,
    Vector,
};

struct Value {
    ValueType type;
};

struct NilValue : public Value {};

struct BoolValue : public Value {
    bool value;
};

struct IntValue : public Value {
    carl_int_t value;
};

}  // namespace carl
#endif