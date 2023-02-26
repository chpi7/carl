#ifndef carl_vm_h
#define carl_vm_h

#include <cstdint>
#include <memory>
#include <unordered_set>

#include "carl/vm/chunk.h"
#include "carl/vm/env.h"

namespace carl {

typedef int64_t carl_stackelem_t;

enum InterpretResult {
    STEP_OK,
    STEP_HALT,
    STEP_ERROR
};

class VM {
   private:
    // Stack
    uint64_t stack_size;
    carl_stackelem_t* sp;
    carl_stackelem_t* stack = nullptr;

    // Program Code
    uint8_t* ip;
    std::unique_ptr<Chunk> chunk;

    // Heap (allocate carl::Value's on the real heap)
    // env is used to lookup addresses based on names
    std::unique_ptr<Env> env;
    // hold on to the names such that the addresses used during runtime will stay valid.
    std::unordered_set<std::shared_ptr<std::string>> names;

   public:
    VM(uint64_t stack_size);
    ~VM();
    void load_chunk(std::unique_ptr<Chunk> chunk);
    InterpretResult step(bool print_trace = false);
    InterpretResult run(bool print_stack_flag = false);
    carl_stackelem_t get_stack_top();
    void print_stack();

   private:
    void free_stack();
    void init_stack();
    inline carl_stackelem_t pop();
    inline carl_stackelem_t peek();
    inline void push(carl_stackelem_t v);
    inline carl_int_t logic_binop(OpCode op, carl_int_t lhs, carl_int_t rhs);
    inline carl_stackelem_t mkbasic();
    inline carl_stackelem_t gtbasic();
    inline void update_value();
};
}  // namespace carl

#endif