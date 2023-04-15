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
    auto mod = generator.take_module();

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
    auto mod = generator.take_module();

    // run
    LLJITWrapper jit;
    jit.load_module(mod);
    auto exprwrapper = jit.lookup("__main");
    ASSERT_TRUE(exprwrapper.has_value());
}

TEST(llvmcodegen, use_host_puts) {
    LLJITWrapper jit;
    std::stringstream ss;
    jit.set_outs(&ss);

    auto lr = jit.lookup_ea("__puts_impl");
    ASSERT_TRUE(lr.has_value());

    auto& x = lr.value();
    auto __puts = x.toPtr<int(__carl_string* s)>();
    std::string test = "hello world from host fn";
    __carl_string s {.len = test.size() + 1, .str = test.c_str()};
    __puts(&s);

    auto outs_content = ss.str();
    ASSERT_EQ(test, outs_content);
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

TEST(llvmcodegen, concat_strings) {
    LLJITWrapper jit;
    std::stringstream ss;
    jit.set_outs(&ss);
    LLVMCodeGenerator cg;
    Parser p;

    std::string src = 
        "let hello = \"hello\";"
        "let world = \"world\";"
        "let hw = hello + \" \" + world + \"!\";"
        "__puts(hw);";
    auto decls = p.parse_r(src);

    cg.generate(*decls);
    auto mod = cg.take_module();

    jit.load_module(mod);
    auto __main = jit.lookup_ea("__main")->toPtr<void()>();
    __main();

    ASSERT_EQ(ss.str(), "hello world!");
}

TEST(llvmcodegen, return_string_from_fn) {
    LLJITWrapper jit;
    std::stringstream ss;
    jit.set_outs(&ss);
    LLVMCodeGenerator cg;
    Parser p;

    std::string src = 
        "fn ret_str() : string {\n"
        "   return \"hello\";\n"
        "}\n"
        "__puts(ret_str());";
    auto decls = p.parse_r(src);

    cg.generate(*decls);
    auto mod = cg.take_module();

    jit.load_module(mod);
    auto __main = jit.lookup_ea("__main")->toPtr<void()>();
    __main();

    ASSERT_EQ(ss.str(), "hello");
}

TEST(llvmcodegen, let_string_while_puts_and_call_fn_through_var) {
    LLJITWrapper jit;
    std::stringstream ss;
    jit.set_outs(&ss);
    LLVMCodeGenerator cg;
    Parser p;

    std::string src = 
        "let s = \"a\";"
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

    ASSERT_EQ(ss.str(), "aaa");
}

TEST(llvmcodegen, call_with_multiple_args_by_value_and_by_reference) {
    LLJITWrapper jit;
    std::stringstream ss;
    jit.set_outs(&ss);

    LLVMCodeGenerator cg;
    Parser p;

    std::string src = 
        "fn foo(x: string, iters: int) : int {"
        "   let i = iters;"
        "   let counter = 0;"
        "   while(i > 0) {"
        "       i = i - 1;"
        "       counter = counter + 1;"
        "       __puts(x);"
        "   }"
        "   return counter;"
        "}"
        "let s = \"foo\";"
        "let counter = foo(s, 3);"
        "__debug(counter);";

    auto decls = p.parse_r(src);
    cg.generate(*decls);
    auto mod = cg.take_module();

    jit.load_module(mod);
    auto __main = jit.lookup_ea("__main")->toPtr<void()>();
    __main();

    auto output = ss.str();
    ASSERT_EQ(output, "foofoofoo");
    ASSERT_EQ(jit.debug_values.size(), 1);
    ASSERT_EQ(jit.debug_values.at(0), 3);
}

TEST(llvmcodegen, pass_fn_as_parameter) {
    LLJITWrapper jit;
    LLVMCodeGenerator cg;
    Parser p;

    std::string src = 
        "fn foo(a: int, f: (int, int : int)) : int {"
        "   return f(a, 1 + a);"
        "}"
        "fn add(a: int, b: int) : int {"
        "   return 2 * a + b;"
        "}"
        "let result = foo(1, add);"
        "__assert(result == 4);";
    auto decls = p.parse_r(src);

    cg.generate(*decls);
    auto mod = cg.take_module();

    jit.load_module(mod);
    auto __main = jit.lookup_ea("__main")->toPtr<void()>();
    __main();
}

TEST(llvmcodegen, closure) {
    LLJITWrapper jit;
    LLVMCodeGenerator cg;
    Parser p;

    std::string src = 
        "let c1 = 1;"
        "let c2 = 2;"
        "fn foo(a: int) : int {"
        "   __debug(c1);"
        "   __debug(c2);"
        "   return c1 + c2 + 3;"
        "}"
        "c1 = 0;"
        "let result = foo(0);";
    auto decls = p.parse_r(src);

    cg.generate(*decls);
    auto mod = cg.take_module();

    jit.load_module(mod);
    auto __main = jit.lookup_ea("__main")->toPtr<void()>();
    __main();

    std::vector<uint64_t> expected {1, 2};
    ASSERT_EQ(jit.debug_values, expected);
}

TEST(llvmcodegen, closure_with_different_captures) {
    LLJITWrapper jit;
    LLVMCodeGenerator cg;
    Parser p;

    std::string src = 
        "fn create_closure(a: int) : (:int) {"
        "   fn foo() : int {"
        "       return a;"
        "   }"
        "   return foo;"
        "}"
        "let c1 = create_closure(1);"
        "let c2 = create_closure(2);"
        "__debug(c1());"
        "__debug(c2());"
        "__debug(c1() + c2());";
    auto decls = p.parse_r(src);

    cg.generate(*decls);
    auto mod = cg.take_module();

    jit.load_module(mod);
    auto __main = jit.lookup_ea("__main")->toPtr<void()>();
    __main();

    std::vector<uint64_t> expected {1, 2, 3};
    ASSERT_EQ(jit.debug_values, expected);
}
}  // namespace