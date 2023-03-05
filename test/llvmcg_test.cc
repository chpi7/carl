#include "carl/llvmcg.h"

#include <gtest/gtest.h>

#include "carl/parser.h"

using namespace carl;

namespace {
TEST(llvmcg, compile_expression) {
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
    std::cout << std::endl;
}
}  // namespace