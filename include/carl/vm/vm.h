#ifndef carl_vm_h
#define carl_vm_h

#include <cstdint>
#include <memory>

#include "carl/vm/chunk.h"

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

    // Heap
    // use the cpp heap for now bc im lazy :)

   public:
    VM(uint64_t stack_size);
    ~VM();
    void load_chunk(std::unique_ptr<Chunk> chunk);
    InterpretResult step();
    InterpretResult run();
    carl_stackelem_t get_stack_top();
    void print_stack();

   private:
    void free_stack();
    void init_stack();
    inline carl_stackelem_t pop();
    inline void push(carl_stackelem_t v);
};
}  // namespace carl

#endif