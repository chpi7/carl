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

TEST(Chunk, set_of_shared_ptr_to_string) {
    auto s1 = std::string("string 1");
    auto s1_d = std::string("string 1");
    auto s2 = std::string("string 2");

    Chunk c(255);
    auto saved1 = c.save_name(s1); 
    auto saved1_d = c.save_name(s1_d); 
    ASSERT_EQ(saved1, saved1_d);

    auto saved2 = c.save_name(s2);
    ASSERT_NE(saved2, saved1);

    auto r1 = c.get_name(s1);
    auto r1_d = c.get_name(s1_d);
    ASSERT_EQ(r1, r1_d);
}
}  // namespace