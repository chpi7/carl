#ifndef carl_memory_h
#define carl_memory_h

#include <fstream>
#include <cstdint>

namespace carl {

typedef int64_t carl_int_t;

enum ValueType {
    Nil,
    Int,  // Basic and Pointer
    Function,
    Closure,
    Vector,
};

struct Value {
    ValueType type;
};

struct NilValue : public Value {};

struct IntValue : public Value {
    carl_int_t value;
};

}  // namespace carl
#endif