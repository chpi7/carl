#include "carl/vm/vm.h"

#include <gtest/gtest.h>

using namespace carl;

namespace {
TEST(VM, add_two) {
    VM vm(1024);

    auto chunk = std::make_unique<Chunk>(255);
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(1);
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(2);
    chunk->write_byte(OP_ADD);
    chunk->write_byte(OP_HALT);

    vm.load_chunk(std::move(chunk));
    vm.step();
    vm.step();
    vm.step();

    auto result = vm.get_stack_top();
    ASSERT_EQ(result, 3);
}

TEST(VM, sub_add_three) {
    VM vm(1024);

    auto chunk = std::make_unique<Chunk>(255);
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

    vm.load_chunk(std::move(chunk));
    auto exit_code = vm.run();
    ASSERT_EQ(exit_code, STEP_HALT);

    auto result = vm.get_stack_top();
    ASSERT_EQ(result, -3);
}

TEST(VM, def_get_var) {
    VM vm(1024);
    const char* name = "x";

    auto chunk = std::make_unique<Chunk>(255);
    // value
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(0xbeef);
    // name
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(reinterpret_cast<carl_int_t>(name));
    // define 
    chunk->write_byte(OP_DEFINE_VAR);

    // load 1
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(reinterpret_cast<carl_int_t>(name));
    chunk->write_byte(OP_GET_VAR);

    chunk->write_byte(OP_HALT);

    // update
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(0xdead);
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(reinterpret_cast<carl_int_t>(name));
    chunk->write_byte(OP_SET_VAR);

    // load 2
    chunk->write_byte(OP_LOADC);
    chunk->write_int_const(reinterpret_cast<carl_int_t>(name));
    chunk->write_byte(OP_GET_VAR);

    chunk->write_byte(OP_HALT);

    chunk->print(std::cout);

    vm.load_chunk(std::move(chunk));

    auto exit_code = vm.run();
    ASSERT_EQ(exit_code, STEP_HALT);
    auto result = vm.get_stack_top();
    ASSERT_EQ(result, 0xbeef);

    exit_code = vm.run();
    ASSERT_EQ(exit_code, STEP_HALT);
    result = vm.get_stack_top();
    ASSERT_EQ(result, 0xdead);
}
}  // namespace