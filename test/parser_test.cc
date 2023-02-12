#include "carl/parser.h"

#include <gtest/gtest.h>

#include <vector>

#include "carl/scanner.h"

using namespace carl;

namespace {

class A {
    public:
    virtual void foo() = 0;
};

class B : public A {
    public:
    void foo() {
        printf("heyho\n");
    }
};

TEST(Parser, IDontUnderstandWhatsGoingOn) {
    A* a = new B;
    a->foo();

    auto aa = std::make_shared<B>();
    aa->foo();

    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    AstNode* bb = new Variable(Token{TOKEN_BANG, nullptr, 0, 0});
    bb->accept(v);

    const char* test_string = "1";
    const char* test_op = "+";
    std::shared_ptr<AstNode> child = std::make_shared<Literal>(Token{TOKEN_DOT, test_string, 1, 0});
    std::shared_ptr<AstNode> bbb = std::make_shared<Binary>(Token{TOKEN_PLUS, test_op, 1, 0}, child, child);
    bbb->accept(v);
}

TEST(Parser, parse_expression_simple) {
    auto scanner = std::make_shared<Scanner>();
    const char* expr_src = "1 + 2 * 3";
    scanner->init(expr_src);

    Parser parser(scanner);

    std::shared_ptr<AstNode> node = parser.expression(); 

    Binary* add = dynamic_cast<Binary*>(node.get());
    Literal* one = dynamic_cast<Literal*>(add->get_lhs().get());
    Binary* mul = dynamic_cast<Binary*>(add->get_rhs().get());
    Literal* two = dynamic_cast<Literal*>(mul->get_lhs().get());
    Literal* three = dynamic_cast<Literal*>(mul->get_rhs().get());


    std::ostringstream ss;
    auto v = new PrintAstNodeVisitor(ss);
    node->accept(v);
    std::cout << ss.str() << std::endl;
}
}  // namespace