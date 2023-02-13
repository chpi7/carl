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

    Parser parser(scanner);
    CodeGenerator generator;
    auto expression = parser.expression();

    expression->accept(&generator);
    
    VM vm(256);
    vm.load_chunk(generator.take_chunk());

    ASSERT_EQ(generator.get_chunk(), nullptr);

    vm.run();
    auto r = vm.get_stack_top();

    ASSERT_EQ(r, 14);
}
}  // namespace