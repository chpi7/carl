#include "carl/parser.h"

#include <gtest/gtest.h>

#include <vector>

#include "carl/ast/print_visitor.h"
#include "carl/ast/ast_printer.h"
#include "carl/scanner.h"

using namespace carl;

namespace {

TEST(Parser, parse_expression_simple) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "1 + 2 * -3";
    std::string expected_print = "(+ 1 (* 2 (- 3)))";
    scanner->init(expr_src);

    Parser parser;
    parser.set_scanner(scanner);

    std::shared_ptr<AstNode> node = parser.expression();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    node->accept(v);

    ASSERT_EQ(ss.str(), expected_print);
    delete v;
}

TEST(Parser, parse_normally_left_assoc) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "a + b + c";
    std::string expected_print = "(+ (+ a b) c)";
    scanner->init(expr_src);

    Parser parser;
    parser.set_scanner(scanner);

    std::shared_ptr<AstNode> node = parser.expression();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    node->accept(v);

    ASSERT_EQ(ss.str(), expected_print);
    delete v;
}

TEST(Parser, parse_assignment_right_assoc) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "a = b = c";
    std::string expected_print = "(= a (= b c))";
    scanner->init(expr_src);

    Parser parser;
    parser.set_scanner(scanner);

    std::shared_ptr<AstNode> node = parser.expression();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    node->accept(v);

    ASSERT_EQ(ss.str(), expected_print);
    delete v;
}

TEST(Parser, parse_expression_logical) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "1 || (1 + 2 == 3)";
    std::string expected_print = "(|| 1 (== (+ 1 2) 3))";
    scanner->init(expr_src);

    Parser parser;
    parser.set_scanner(scanner);

    std::shared_ptr<AstNode> node = parser.expression();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    node->accept(v);

    ASSERT_EQ(ss.str(), expected_print);
    delete v;
}

TEST(Parser, parse_expression_grouping) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "0 + (1.3 + 2) * -3";
    std::string expected_print = "(+ 0 (* (+ 1.3 2) (- 3)))";
    scanner->init(expr_src);

    Parser parser;
    parser.set_scanner(scanner);

    std::shared_ptr<AstNode> node = parser.expression();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    node->accept(v);

    ASSERT_EQ(ss.str(), expected_print);
    delete v;
}

TEST(Parser, parse_expression_associativity) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "0 + 1 + 2 * 3";
    std::string expected_print = "(+ (+ 0 1) (* 2 3))";
    scanner->init(expr_src);

    Parser parser;
    parser.set_scanner(scanner);

    std::shared_ptr<AstNode> node = parser.expression();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    node->accept(v);

    ASSERT_EQ(ss.str(), expected_print);
    delete v;
}

TEST(Parser, parse_everything) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "1 + 2 * 3 < 4 && true || false";
    std::string expected_print = "(|| (&& (< (+ 1 (* 2 3)) 4) true) false)";
    scanner->init(expr_src);

    Parser parser;
    parser.set_scanner(scanner);

    std::shared_ptr<AstNode> node = parser.expression();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    node->accept(v);

    ASSERT_EQ(ss.str(), expected_print);
    delete v;
}

TEST(Parser, parse_nil) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "nil || nil";
    std::string expected_print = "(|| nil nil)";
    scanner->init(expr_src);

    Parser parser;
    parser.set_scanner(scanner);

    std::shared_ptr<AstNode> node = parser.expression();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    node->accept(v);

    ASSERT_EQ(ss.str(), expected_print);
    delete v;
}

TEST(Parser, parse_exprstmt) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "1 + 2;";
    std::string expected_print = "(+ 1 2);";
    scanner->init(expr_src);

    Parser parser;
    parser.set_scanner(scanner);

    std::shared_ptr<AstNode> node = parser.statement();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    node->accept(v);

    ASSERT_EQ(ss.str(), expected_print);
    delete v;
}

TEST(Parser, parse_letdecl) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "let x = 1 + 2;";
    std::string expected_print = "let x = (+ 1 2);";
    scanner->init(expr_src);

    Parser parser;
    parser.set_scanner(scanner);

    std::shared_ptr<AstNode> node = parser.declaration();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    node->accept(v);

    ASSERT_EQ(ss.str(), expected_print);
    delete v;
}

TEST(Parser, parse_while_stmt) {
    auto scanner = std::make_shared<Scanner>();
    const char* src_string = "let i = 1; while (i < 10) {1 + 2;}";
    std::string expected_print = "let i = 1;while (< i 10) {(+ 1 2);}";
    scanner->init(src_string);

    Parser parser;
    parser.set_scanner(scanner);

    std::vector<std::shared_ptr<AstNode>> decls = parser.parse();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    decls.front()->accept(v);
    decls.at(1)->accept(v);

    ASSERT_EQ(ss.str(), expected_print);
    delete v;
}

TEST(Parser, parse_assign_to_call_is_error) {
    Parser parser;
    std::string src = "fn foo() {} foo() = 1234;";

    ParseResult r = parser.parse_r(src);
    ASSERT_FALSE(r);
}

TEST(Parser, parse_call_in_binop) {
    auto scanner = std::make_shared<Scanner>();
    const char* src_string = 
    "let b = foo(a, 123) + 2;";
    scanner->init(src_string);

    Parser parser;
    parser.set_scanner(scanner);

    std::vector<std::shared_ptr<AstNode>> decls = parser.parse();

    ASSERT_EQ(decls.size(), 1);
}

TEST(Parser, parse_nested_fndecls) {
    std::string src_string = 
    "fn foo (a: int, b: int) {\n"
        "fn bar() : string {\n"
        "   let a = foo(1, 2.);\n"
        "}\n"
        "let bar_ref = bar;\n"
        "let sq = a * a + bar_ref();\n"
        "return sq + b;\n"
    "}\n"
    "let a = 2 + 3.;\n"
    "let result_foo = foo(a, 3);\n"
    "let check = result_foo == 7;";
    
    Parser parser;
    auto result = parser.parse_r(src_string);
    ASSERT_TRUE(result);

    auto decls = *result;
    AstPrinter printer(std::cout);
    for (auto& node : decls) {
        printer.print(node.get());
    }

    ASSERT_EQ(decls.size(), 4);
}

TEST(Parser, expect_name_not_found) {
    Parser p;
    std::string src = "let a = b + 1;";

    auto r = p.parse_r(src);
    ASSERT_FALSE(r);
}

TEST(Parser, var_not_found_anymore) {
    Parser p;
    std::string src = 
    "let a = 1;"
    "while (a < 3) {"
    "   let b = a + 1;"
    "}"
    "let c = b;";
    
    auto r = p.parse_r(src);
    ASSERT_FALSE(r);
}

TEST(Parser, fn_decl_found) {
    Parser p;
    std::string src = 
    "fn foo(a: int) {"
    "   let danger = foo(1);"
    "}"
    "let call = foo(0);";
    
    auto r = p.parse_r(src);
    ASSERT_TRUE(r);
}

TEST(Parser, fn_decl_not_found) {
    Parser p;
    std::string src = 
    "fn foo(a: int) {\n"
    "   let danger = baz(1);\n"
    "}\n"
    "let bar = bar();\n"
    "let call = foo(0);";
    
    auto r = p.parse_r(src);
    ASSERT_FALSE(r);
}
}  // namespace