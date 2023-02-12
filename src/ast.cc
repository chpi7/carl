#include <iostream>
#include "carl/ast.h"

static void print_token(char* buf, carl::Token* token) {
    sprintf(buf, "%.*s", token->length, token->start);
}

namespace carl {

void PrintAstNodeVisitor::visit_binary(Binary* node) {
    char buf[255] { 0 };
    print_token(buf, node->get_op());

    os << "(";
    os << buf << " ";
    node->get_lhs()->accept(this);
    os << " ";
    node->get_rhs()->accept(this);
    os << ")";
}

void PrintAstNodeVisitor::visit_unary(Unary* node) {
    os << "(";
    char buf[255] { 0 };
    print_token(buf, node->get_op());
    os << buf;
    node->get_value()->accept(this);
    os << ")";
}

void PrintAstNodeVisitor::visit_variable(Variable* node) {
    char buf[255] { 0 };
    print_token(buf, node->get_name());
    os << buf;
}

void PrintAstNodeVisitor::visit_literal(Literal* node) {
    char buf[255] { 0 };
    print_token(buf, node->get_value());
    os << buf;
}


void Binary::accept(AstNodeVisitor* visitor) { visitor->visit_binary(this); }
void Unary::accept(AstNodeVisitor* visitor) { visitor->visit_unary(this); }
void Variable::accept(AstNodeVisitor* visitor) { visitor->visit_variable(this); }
void Literal::accept(AstNodeVisitor* visitor) { visitor->visit_literal(this); }
}  // namespace carl