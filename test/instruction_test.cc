#include "carl/vm/instruction.h"

#include <gtest/gtest.h>

using namespace carl;

namespace {
TEST(Chunk, write_and_print) {
    Chunk chunk(255);
    for (int i = 0; i < 2; ++i) {
        chunk.write_opcode(OP_NOP);
        chunk.write_opcode(OP_ADD);
        chunk.write_opcode(OP_POP);
        chunk.write_opcode(OP_LOADC);
        chunk.write_int_const(0xbeef);
        chunk.write_opcode(OP_HALT);
    }

    std::ostringstream os;
    chunk.print(os);
    auto result = os.str();
    // std::cout << result << std::endl;

    ASSERT_EQ(chunk.get_write_offset(), 26);
}
}  // namespace