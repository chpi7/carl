#include <gtest/gtest.h>

#include "carl/parser.h"
#include "carl/jit2/carljit.h"
#include "carl/jit2/codegen2.h"
#include "carl/ast/ast_printer.h"

#include "carl/jit2/runtime_types.h"

using namespace carl;

namespace {
TEST(codegen2, string_fndecl_with_capture) {
    CarlJIT jit;
    Parser p;
    AstPrinter printer(std::cout);

    Codegen2 cg;
    cg.init("main");

    std::string src = ""
        "let one = 1;"
        "fn foo(a: int): int {"
        "   return a + one;"
        "}"
        "one = one + 1;"
        "return foo(41);";
    auto decls = p.parse_r(src, false);
    for (auto& d : *decls) printer.print(d.get());

    auto module = cg.generate(*decls);

    jit.load_module(module);
    auto __main = jit.lookup_ea("__carl_main")->toPtr<uint64_t()>();
    uint64_t result = __main();
    ASSERT_EQ(result, 42);
}

TEST(codegen2, string_letdecl) {
    CarlJIT jit;
    Parser p;
    AstPrinter printer(std::cout);

    Codegen2 cg;
    cg.init("main");

    std::string src = ""
        "let a = 40;"
        "let b = 2;"
        "return a + b;";
    auto decls = p.parse_r(src, false);
    for (auto& d : *decls) printer.print(d.get());

    auto module = cg.generate(*decls);

    jit.load_module(module);
    auto __main = jit.lookup_ea("__carl_main")->toPtr<uint64_t()>();
    uint64_t result = __main();
    ASSERT_EQ(result, 42);
}

TEST(codegen2, string_concat) {
    CarlJIT jit;
    Parser p;
    AstPrinter printer(std::cout);

    Codegen2 cg;
    cg.init("main");

    std::string src = "return \"hello \" + \"world!\";";
    auto decls = p.parse_r(src, false);
    for (auto& d : *decls) printer.print(d.get());

    auto module = cg.generate(*decls);

    jit.load_module(module);
    auto __main = jit.lookup_ea("__carl_main")->toPtr<crt_string*()>();
    crt_string* result = __main();
    ASSERT_STREQ(result->data, "hello world!");
    ASSERT_EQ(result->len, 13);
}

TEST(codegen2, string_create) {
    CarlJIT jit;
    Parser p;
    AstPrinter printer(std::cout);

    Codegen2 cg;
    cg.init("main");

    std::string src = "return \"hello world!\";";
    auto decls = p.parse_r(src, false);
    for (auto& d : *decls) printer.print(d.get());

    auto module = cg.generate(*decls);

    jit.load_module(module);
    auto __main = jit.lookup_ea("__carl_main")->toPtr<crt_string*()>();
    crt_string* result = __main();
    ASSERT_STREQ(result->data, "hello world!");
    ASSERT_EQ(result->len, 13);
}

TEST(codegen2, basic_expression_add) {
    CarlJIT jit;
    Parser p;
    AstPrinter printer(std::cout);

    Codegen2 cg;
    cg.init("main");

    std::string src = "return 1 + 2;";
    auto decls = p.parse_r(src, false);
    for (auto& d : *decls) printer.print(d.get());

    auto module = cg.generate(*decls);

    jit.load_module(module);
    auto __main = jit.lookup_ea("__carl_main")->toPtr<uint64_t()>();
    ASSERT_EQ(__main(), 3);
}
}