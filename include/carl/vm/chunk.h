#ifndef carl_chunk_h
#define carl_chunk_h

#include <fstream>
#include <vector>

#include "carl/vm/memory.h"
#include "carl/vm/instruction.h"

namespace carl {

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