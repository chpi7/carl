#ifndef carl_memory_h
#define carl_memory_h

#include <cstdint>

namespace carl {

typedef int64_t carl_int_t;
#define CARL_NIL INT64_MIN
#define CARL_TRUE (static_cast<carl_int_t>(1))
#define CARL_FALSE (static_cast<carl_int_t>(0))

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