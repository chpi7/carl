#include <gtest/gtest.h>

#include "carl/llvm/codegen.h"
#include "carl/parser.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

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

TEST(llvmcodegen, basic_lljit_functionality_check) {
    llvm::ExitOnError exitErr;

    LLVMCodeGenerator gen;
    gen.create_dummy_function();
    auto mod = gen.take_module();

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    llvm::orc::LLJITBuilder builder;

    auto j = exitErr(llvm::orc::LLJITBuilder().create());
    exitErr(j->addIRModule(std::move(mod)));

    auto x = j->lookup("__main").get().toPtr<int32_t()>();
    int32_t res = x();

    ASSERT_EQ(res, 1337);
}
}  // namespace