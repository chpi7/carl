#ifndef carl_chunk_h
#define carl_chunk_h

#include <fstream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "carl/vm/instruction.h"
#include "carl/vm/memory.h"

namespace carl {

// Represents a chunk of executable memory.
class Chunk {
   private:
    uint8_t* memory;
    // Points to the next writable memory location.
    uint8_t* write_pos;
    // Size of memory.
    uint64_t size;
    // make sure names stay valid so we can give them to the vm.
    std::unordered_map<std::string, std::shared_ptr<std::string>> names;

   public:
    Chunk(uint64_t size);
    ~Chunk();
    void write_byte(OpCode opcode);
    void write_value_type(ValueType type);
    void write_int_const(carl_int_t c);
    void print(std::ostream& os);
    int print_single(std::ostream& os, uint8_t* pos);
    uint8_t* get_memory();
    std::shared_ptr<std::string> save_name(std::string name);
    std::shared_ptr<std::string> get_name(std::string name);
    std::unordered_map<std::string, std::shared_ptr<std::string>>& get_names();

    // for testing only.
    uint64_t get_write_offset();

};
}  // namespace carl

#endif