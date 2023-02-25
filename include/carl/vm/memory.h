#ifndef carl_memory_h
#define carl_memory_h

#include <cstdint>

namespace carl {

typedef int64_t carl_int_t;
#define CARL_NIL INT64_MIN
#define CARL_TRUE (static_cast<carl_int_t>(1))
#define CARL_FALSE (static_cast<carl_int_t>(0))

enum ValueType {
    Basic,
    Function,
    Closure,
    Vector,
};

struct Value {
    ValueType type;
};

struct BasicValue : public Value {
    carl_int_t value;
};

static size_t get_size(ValueType type) {
    switch (type) {
        case Basic: return sizeof(BasicValue);
        default: return 0;
    }
};

}  // namespace carl
#endif