#include "carl/ast/print_visitor.h"

using namespace carl;

char stringify_buffer[255]{0};
static void stringify_token(char* buf, carl::Token* token) {
    sprintf(buf, "%.*s", token->length, token->start);
}

void PrintAstNodeVisitor::visit_binary(Binary* node) {
    os << "(";
    stringify_token(stringify_buffer, node->get_op());
    os << stringify_buffer << " ";
    node->get_lhs()->accept(this);
    os << " ";
    node->get_rhs()->accept(this);
    os << ")";
}

void PrintAstNodeVisitor::visit_unary(Unary* node) {
    os << "(";
    stringify_token(stringify_buffer, node->get_op());
    os << stringify_buffer << " ";
    node->get_value()->accept(this);
    os << ")";
}

void PrintAstNodeVisitor::visit_variable(Variable* node) {
    stringify_token(stringify_buffer, node->get_name());
    os << stringify_buffer;
}

void PrintAstNodeVisitor::visit_literal(Literal* node) {
    stringify_token(stringify_buffer, node->get_value());
    os << stringify_buffer;
}

void PrintAstNodeVisitor::visit_string(String* node) {
    stringify_token(stringify_buffer, node->get_value());
    os << stringify_buffer;
}

void PrintAstNodeVisitor::visit_number(Number* node) {
    stringify_token(stringify_buffer, node->get_value());
    os << stringify_buffer;
}