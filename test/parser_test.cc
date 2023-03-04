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

TEST(Parser, parse_letstmt) {
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

TEST(Parser, parse_block) {
    auto scanner = std::make_shared<Scanner>();
    const char* src_string = "{"
    "x + 1;"
    "y + 2;"
    "}";
    std::string expected_print = "{(+ x 1);, (+ y 2);}";
    scanner->init(src_string);

    Parser parser;
    parser.set_scanner(scanner);

    std::vector<std::shared_ptr<AstNode>> decls = parser.parse();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    decls.front()->accept(v);

    ASSERT_EQ(ss.str(), expected_print);
    delete v;
}

TEST(Parser, parse_while_stmt) {
    auto scanner = std::make_shared<Scanner>();
    const char* src_string = "while (i < 10) {1 + 2;}";
    std::string expected_print = "while (< i 10) {(+ 1 2);}";
    scanner->init(src_string);

    Parser parser;
    parser.set_scanner(scanner);

    std::vector<std::shared_ptr<AstNode>> decls = parser.parse();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    decls.front()->accept(v);

    ASSERT_EQ(ss.str(), expected_print);
    delete v;
}

TEST(Parser, parse_fndecl) {
    auto scanner = std::make_shared<Scanner>();
    const char* src_string = 
    "fn foo (a: int, b: int) {"
        "let sq = a * a;"
        "return sq + b;"
    "}"
    "let a = 2;"
    "let result_foo = foo(a, 3);" // TODO: implement call expression.
    "let check = result_foo == 7;";
    scanner->init(src_string);

    Parser parser;
    parser.set_scanner(scanner);

    std::vector<std::shared_ptr<AstNode>> decls = parser.parse();

    ASSERT_EQ(decls.size(), 4);

    AstPrinter printer(std::cout);
    for (auto& decl : decls) {
        printer.print(decl.get());
    }
}
}  // namespace