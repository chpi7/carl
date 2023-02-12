#ifndef carl_instruction_h
#define carl_instruction_h

#include <vector>

#include "carl/vm/memory.h"

namespace carl {
enum OpCode {
    OP_NOP,
    OP_MKBASIC,
    OP_GTBASIC,
    OP_LOADC,
    OP_POP,
    OP_DUP,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_NEG,
    OP_EQ,
    OP_LE,
    OP_LEQ,
    OP_JUMP,
    OP_JUMPZ,
    OP_HALT,
};

// Represents a chunk of executable memory.
class Chunk {
    uint8_t* memory;
    // Points to the next writable memory location.
    uint8_t* write_pos;
    // Size of memory.
    uint64_t size;

   public:
    Chunk(uint64_t size);
    ~Chunk();
    void write_byte(OpCode opcode);
    void write_int_const(carl_int_t c);
    void print(std::ostream& os);
    uint8_t* get_memory();

    // for testing only.
    uint64_t get_write_offset();
};
}  // namespace carl

#endif