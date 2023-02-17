#include "carl/codegen.h"

#include <iostream>

#include <gtest/gtest.h>

#include "carl/parser.h"
#include "carl/ast/print_visitor.h"
#include "carl/vm/vm.h"

using namespace carl;

namespace {
TEST(CodeGen, SimpleCalc) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "2 + 3 * 4";
    scanner->init(expr_src);

    Parser parser;
    CodeGenerator generator;
    parser.set_scanner(scanner);
    auto expression = parser.expression();

    generator.generate(expression);
    
    VM vm(256);
    vm.load_chunk(generator.take_chunk());

    ASSERT_EQ(generator.get_chunk(), nullptr);

    vm.run();
    auto r = vm.get_stack_top();

    ASSERT_EQ(r, 14);
}

TEST(CodeGen, expr_stmt) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "2 + 3 * 4;";
    scanner->init(expr_src);

    Parser parser;
    CodeGenerator generator;
    parser.set_scanner(scanner);
    auto expression = parser.expression();

    generator.generate(expression);
    
    VM vm(256);
    vm.load_chunk(generator.take_chunk());

    ASSERT_EQ(generator.get_chunk(), nullptr);

    vm.run();
    auto r = vm.get_stack_top();

    ASSERT_EQ(r, 14);
}

TEST(CodeGen, logic_f_o_t_a_t_o_nil) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "false || (true && true) || nil";
    scanner->init(expr_src);

    Parser parser;
    CodeGenerator generator;
    parser.set_scanner(scanner);
    auto expression = parser.expression();

    generator.generate(expression);
    
    VM vm(256);
    vm.load_chunk(generator.take_chunk());

    ASSERT_EQ(generator.get_chunk(), nullptr);

    vm.run();
    auto r = vm.get_stack_top();

    ASSERT_EQ(r, CARL_TRUE);
}

TEST(CodeGen, logic_false_and_nil) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "!(false && nil)";
    scanner->init(expr_src);

    Parser parser;
    CodeGenerator generator;
    parser.set_scanner(scanner);
    auto expression = parser.expression();

    generator.generate(expression);
    
    VM vm(256);
    vm.load_chunk(generator.take_chunk());

    ASSERT_EQ(generator.get_chunk(), nullptr);

    vm.run();
    auto r = vm.get_stack_top();

    ASSERT_EQ(r, CARL_TRUE);
}

TEST(CodeGen, logic_false_or_nil) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "!(false || nil)";
    scanner->init(expr_src);

    Parser parser;
    CodeGenerator generator;
    parser.set_scanner(scanner);
    auto expression = parser.expression();

    generator.generate(expression);
    
    VM vm(256);
    vm.load_chunk(generator.take_chunk());

    ASSERT_EQ(generator.get_chunk(), nullptr);

    vm.run();
    auto r = vm.get_stack_top();

    ASSERT_EQ(r, CARL_NIL);
}

TEST(CodeGen, logic_true_and_false) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "true && false";
    scanner->init(expr_src);

    Parser parser;
    CodeGenerator generator;
    parser.set_scanner(scanner);
    auto expression = parser.expression();

    generator.generate(expression);
    
    VM vm(256);
    vm.load_chunk(generator.take_chunk());

    ASSERT_EQ(generator.get_chunk(), nullptr);

    vm.run();
    auto r = vm.get_stack_top();

    ASSERT_EQ(r, CARL_FALSE);
}
}  // namespace