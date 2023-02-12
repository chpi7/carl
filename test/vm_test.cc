#include "carl/vm/vm.h"

#include <gtest/gtest.h>

using namespace carl;

namespace {
TEST(VM, add_two) {
    VM vm(1024);

    auto chunk = std::make_shared<Chunk>(255);
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(1);
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(2);
    chunk->write_byte(OP_ADD);
    chunk->write_byte(OP_HALT);

    vm.load_chunk(chunk);
    vm.step();
    vm.step();
    vm.step();

    auto result = vm.get_stack_top();
    ASSERT_EQ(result, 3);
}

TEST(VM, sub_add_three) {
    VM vm(1024);

    auto chunk = std::make_shared<Chunk>(255);
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(1);
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(2);
    chunk->write_byte(OP_SUB);
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(4);
    chunk->write_byte(OP_ADD);
    chunk->write_byte(OP_NEG);
    chunk->write_byte(OP_HALT);

    vm.load_chunk(chunk);
    auto exit_code = vm.run();
    ASSERT_EQ(exit_code, STEP_HALT);

    auto result = vm.get_stack_top();
    ASSERT_EQ(result, -3);
}
}  // namespace