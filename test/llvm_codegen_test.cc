#include <gtest/gtest.h>

#include "carl/llvm/codegen.h"
#include "carl/parser.h"
#include "carl/llvm/jit.h"

using namespace carl;

namespace {
TEST(llvmcodegen, compile_expression) {
    auto scanner = std::make_shared<Scanner>();
    const char* src_string = "1 + 2 * 3 == 7;";
    scanner->init(src_string);

    Parser parser;
    parser.set_scanner(scanner);

    std::vector<std::shared_ptr<AstNode>> decls = parser.parse();
    auto expr = decls.front();

    LLVMCodeGenerator generator;
    llvm::Value* value = generator.do_visit(expr.get());
    value->print(llvm::outs());
    llvm::outs() << "\n";
}

TEST(llvmcodegen, basic_expr_compile_eval) {
    // compile
    auto scanner = std::make_shared<Scanner>();
    const char* src_string = "40 + 2 == 43";
    scanner->init(src_string);

    Parser parser;
    parser.set_scanner(scanner);

    auto expr = parser.expression();

    LLVMCodeGenerator generator;
    generator.generate_eval(expr.get());
    auto mod = generator.take_module();

    // run
    LLJITWrapper jit;
    jit.load_module(mod);
    auto exprwrapper = jit.lookup("__expr_wrapper");
    ASSERT_TRUE(exprwrapper.has_value());
    
    auto f = reinterpret_cast<bool(*)()>(exprwrapper.value());
    ASSERT_FALSE(f());
}
}  // namespace