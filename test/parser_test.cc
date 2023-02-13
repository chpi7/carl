#include "carl/parser.h"

#include <gtest/gtest.h>

#include <vector>

#include "carl/ast/print_visitor.h"
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
}

TEST(Parser, parse_letstmt) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "let x = 1 + 2;";
    std::string expected_print = "let x = (+ 1 2);";
    scanner->init(expr_src);

    Parser parser;
    parser.set_scanner(scanner);

    std::shared_ptr<AstNode> node = parser.statement();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    node->accept(v);

    ASSERT_EQ(ss.str(), expected_print);
}
}  // namespace