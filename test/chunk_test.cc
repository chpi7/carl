#include "carl/vm/chunk.h"

#include <gtest/gtest.h>

using namespace carl;

namespace {
TEST(Chunk, write_and_print) {
    Chunk chunk(255);
    for (int i = 0; i < 2; ++i) {
        chunk.write_byte(OP_NOP);
        chunk.write_byte(OP_ADD);
        chunk.write_byte(OP_POP);
        chunk.write_byte(OP_LOADC);
        chunk.write_int_const(0xbeef + i);
        chunk.write_byte(OP_DUP);
    }

    std::ostringstream os;
    chunk.print(os);
    auto result = os.str();
    // std::cout << result << std::endl;
    std::string expected =
        "   0: NOP\n"
        "   1: ADD\n"
        "   2: POP\n"
        "   3: LOADC 000000000000beef\n"
        "   c: DUP\n"
        "   d: NOP\n"
        "   e: ADD\n"
        "   f: POP\n"
        "  10: LOADC 000000000000bef0\n"
        "  19: DUP\n";

    ASSERT_EQ(chunk.get_write_offset(), 26);
    ASSERT_EQ(os.str(), expected);
}
}  // namespace