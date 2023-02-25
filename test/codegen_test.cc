#include "carl/codegen.h"

#include <gtest/gtest.h>

#include <iostream>

#include "carl/ast/ast_printer.h"
#include "carl/ast/print_visitor.h"
#include "carl/parser.h"
#include "carl/vm/vm.h"

using namespace carl;

namespace {
TEST(CodeGen, simple_calc) {
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
    auto r = reinterpret_cast<BasicValue*>(vm.get_stack_top());

    ASSERT_EQ(r->type, ValueType::Basic);
    ASSERT_EQ(r->value, 14);
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
    generator.get_chunk()->write_byte(OP_GTBASIC);

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
    generator.get_chunk()->write_byte(OP_GTBASIC);

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
    generator.get_chunk()->write_byte(OP_GTBASIC);

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
    generator.get_chunk()->write_byte(OP_GTBASIC);

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
    generator.get_chunk()->write_byte(OP_GTBASIC);

    VM vm(256);
    vm.load_chunk(generator.take_chunk());

    ASSERT_EQ(generator.get_chunk(), nullptr);

    vm.run();
    auto r = vm.get_stack_top();

    ASSERT_EQ(r, CARL_FALSE);
}

TEST(CodeGen, letstmt) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src =
        "let x = 1;"
        "x + 1;";
    scanner->init(expr_src);

    Parser parser;
    CodeGenerator generator;
    parser.set_scanner(scanner);
    auto decl_list = parser.parse();
    ASSERT_EQ(decl_list.size(), 2);

    generator.generate(decl_list);

    VM vm(256);
    vm.load_chunk(generator.take_chunk());

    ASSERT_EQ(generator.get_chunk(), nullptr);

    for (int i = 0; i < 11; ++i) vm.step();
    auto r = vm.get_stack_top();
    ASSERT_EQ(r, 2);

    auto code = vm.run();
    ASSERT_EQ(code, STEP_HALT);
}

TEST(CodeGen, single_var_expr_stmt) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "x;";
    scanner->init(expr_src);

    Parser parser;
    CodeGenerator generator;
    parser.set_scanner(scanner);
    auto decl_list = parser.parse();

    generator.generate(decl_list);

    VM vm(256);
    vm.load_chunk(generator.take_chunk());

    auto code = vm.run();
    ASSERT_EQ(code, STEP_ERROR);
}

TEST(CodeGen, let_and_set) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src =
        "let x = 1;"
        "x = 2;"
        "x;";
    scanner->init(expr_src);

    Parser parser;
    CodeGenerator generator;
    parser.set_scanner(scanner);
    auto decl_list = parser.parse();
    ASSERT_EQ(decl_list.size(), 3);

    generator.generate(decl_list);

    VM vm(256);
    vm.load_chunk(generator.take_chunk());

    ASSERT_EQ(generator.get_chunk(), nullptr);

    for (int i = 0; i < 12; ++i) vm.step();
    BasicValue* r = reinterpret_cast<BasicValue*>(vm.get_stack_top());
    ASSERT_EQ(r->type, ValueType::Basic);
    ASSERT_EQ(r->value, 2);

    auto code = vm.run();
    ASSERT_EQ(code, STEP_HALT);
}
}  // namespace