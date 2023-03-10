#include <gtest/gtest.h>

#include <memory>

#include "carl/ast/ast_printer.h"
#include "carl/llvm/codegen.h"
#include "carl/llvm/jit.h"
#include "carl/parser.h"

using namespace carl;

namespace {
TEST(llvmcodegen, compile_expression) {
    Parser parser;
    std::string src = "1 + 2 * 3 == 7;";
    auto decls = parser.parse(src);
    auto expr = decls.front();

    LLVMCodeGenerator generator;
    llvm::Value* value =
        generator.do_visit(std::reinterpret_pointer_cast<Expression>(expr));
    value->print(llvm::outs());
    llvm::outs() << "\n";
}

TEST(llvmcodegen, basic_expr_compile_eval) {
    // compile
    Parser p;
    std::string src = "40 + 2 == 43;";
    auto decls = p.parse(src);
    auto expr = decls.front();

    LLVMCodeGenerator generator;
    generator.generate_eval(std::reinterpret_pointer_cast<Expression>(expr));
    auto mod = generator.take_module();

    // run
    LLJITWrapper jit;
    jit.load_module(mod);
    auto exprwrapper = jit.lookup("__expr_wrapper");
    ASSERT_TRUE(exprwrapper.has_value());

    auto f = reinterpret_cast<bool (*)()>(exprwrapper.value());
    ASSERT_FALSE(f());
}

TEST(llvmcodegen, let_decls_and_assign) {
    std::string src =
        "let x = 123;"
        "x = x + 1;";

    // compile
    Parser p;
    auto decls = p.parse(src);
    AstPrinter printer(std::cout);
    for (auto& d : decls) {
        printer.print(d.get());
    }

    LLVMCodeGenerator generator;
    generator.generate(decls);
    auto mod = generator.take_module();

    // run
    LLJITWrapper jit;
    jit.load_module(mod);
    auto exprwrapper = jit.lookup("__main");
    ASSERT_TRUE(exprwrapper.has_value());
}
}  // namespace