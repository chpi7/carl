#include <gtest/gtest.h>

#include <memory>

#include <cstdio>

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

    LLVMCodeGenerator generator;
    generator.generate(decls);
    auto mod = generator.take_module();

    // run
    LLJITWrapper jit;
    jit.load_module(mod);
    auto exprwrapper = jit.lookup("__main");
    ASSERT_TRUE(exprwrapper.has_value());
}

TEST(llvmcodegen, while_stmt) {
    std::string src =
        "let x = 0;"
        "while (x < 42) { x = x + 2; } ";

    // compile
    Parser p;
    auto decls = p.parse(src);

    LLVMCodeGenerator generator;
    generator.generate(decls);
    auto mod = generator.take_module(true);

    // run
    LLJITWrapper jit;
    jit.load_module(mod);
    auto exprwrapper = jit.lookup("__main");
    ASSERT_TRUE(exprwrapper.has_value());
    auto __main = (void(*)())(exprwrapper.value());
    __main();
}

TEST(llvmcodegen, string) {
    std::string src = "let s = \"test\";";

    // compile
    Parser p;
    auto decls = p.parse(src);

    LLVMCodeGenerator generator;
    generator.generate(decls);
    auto mod = generator.take_module(true);

    // run
    LLJITWrapper jit;
    jit.load_module(mod);
    auto exprwrapper = jit.lookup("__main");
    ASSERT_TRUE(exprwrapper.has_value());
}

int VAL = 0;
extern "C" {
    void set_val() {
        VAL = 1;
    }

    int inc_val(int x) {
        VAL += x;
        return VAL;
    }
}

TEST(llvmcodegen, register_and_call_host_functions) {
    LLJITWrapper jit;

    jit.register_host_function("set_val", (void*)(set_val));
    jit.register_host_function("inc_val", (void*)(inc_val));
    jit.register_host_function("printf", (void*)(printf));
    auto pf = jit.lookup_ea("printf").value().toPtr<int(const char*, ...)>();
    pf("test %i\n", 1);

    LLVMCodeGenerator gen;
    gen.generate_dummy();
    auto m = gen.take_module();
    jit.load_module(m);

    auto r = jit.lookup_ea("__main");
    auto __main = r.value().toPtr<void()>();

    VAL = 0;
    __main();
    ASSERT_EQ(VAL, 3);
}

TEST(llvmcodegen, use_host_puts) {
    LLJITWrapper jit;

    auto lr = jit.lookup_ea("__puts_impl");
    ASSERT_TRUE(lr.has_value());

    auto& x = lr.value();
    auto __puts = x.toPtr<int(__carl_string* s)>();
    std::string test = "hello world from host fn";
    __carl_string s {.len = test.size() + 1, .str = test.c_str()};
    __puts(&s);
}

TEST(llvmcodegen, simple_builtin_call) {
    LLJITWrapper jit;
    LLVMCodeGenerator cg;
    Parser p;

    std::string src = 
        "let s = \"hello world\";"
        "__puts(s);";

    auto decls = p.parse_r(src);

    cg.generate(*decls);
    auto mod = cg.take_module();

    jit.load_module(mod);
    auto __main = jit.lookup_ea("__main")->toPtr<void()>();
    __main();
}

TEST(llvmcodegen, let_string_while_puts) {
    LLJITWrapper jit;
    LLVMCodeGenerator cg;
    Parser p;

    std::string src = 
        "let s = \"Hello World from carl!\";"
        "let c = 3;"
        "while (c > 0) {"
        "   let tmp = s;"
        "   let foo = __puts;"
        "   foo(tmp);"
        "   c = c - 1;"
        "}";
    auto decls = p.parse_r(src);

    cg.generate(*decls);
    auto mod = cg.take_module();

    jit.load_module(mod);
    auto __main = jit.lookup_ea("__main")->toPtr<void()>();
    __main();
}

TEST(llvmcodegen, call_with_multiple_args_by_value_and_by_reference) {
    LLJITWrapper jit;
    LLVMCodeGenerator cg;
    Parser p;

    std::string src = 
        "fn foo(x: string, iters: int) {"
        "   let i = iters;"
        "   while(i > 0) {"
        "       i = i - 1;"
        "       __puts(x);"
        "   }"
        "}"
        "let s = \"foo\";"
        "foo(s, 3);";
    auto decls = p.parse_r(src);

    cg.generate(*decls);
    auto mod = cg.take_module();

    jit.load_module(mod);
    auto __main = jit.lookup_ea("__main")->toPtr<void()>();
    __main();
}
}  // namespace