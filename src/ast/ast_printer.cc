#include "carl/ast/ast_printer.h"

using namespace carl;

void AstPrinter::visit_invalid(Invalid* invalid){
    write_indent();
    os << "invalid" << std::endl;
}

void AstPrinter::visit_letstmt(LetStmt* letstmt){
    write_indent();
    os << "letStmt\n";
    indent++;
    write_indent();
    os << "name = " << std::string(letstmt->get_name().start, letstmt->get_name().length);
    os << "\n";
    write_indent();
    os << "initializer =\n";
    indent++;
    letstmt->get_initializer()->accept(this);
    indent--;
    indent--;
}

void AstPrinter::visit_exprstmt(ExprStmt* exprstmt){
    write_indent();
    os << "exprStmt\n";
    indent++;
    write_indent();
    os << "expr =\n";
    exprstmt->get_expr()->accept(this);
    indent--;
}

void AstPrinter::visit_binary(Binary* node){
    write_indent();
    os << "binary\n";

    indent++;
    write_indent();
    os << "op = " << std::string(node->get_op().start, node->get_op().length);
    os << "\n";

    write_indent();
    os << "lhs = ";
    os << "\n";
    indent++;
    node->get_lhs()->accept(this);
    os << "\n";
    indent--;

    write_indent();
    os << "rhs = ";
    os << "\n";
    indent++;
    node->get_rhs()->accept(this);
    indent--;

    indent--;
}

void AstPrinter::visit_unary(Unary* node){
    write_indent();
    os << "unary\n";

    indent++;
    write_indent();
    os << "op = " << std::string(node->get_op().start, node->get_op().length);
    os << "\n";

    write_indent();
    os << "operand = ";
    os << "\n";
    indent++;
    node->get_operand()->accept(this);
    indent--;

    indent--;
}

void AstPrinter::visit_variable(Variable* node){
    write_indent();
    os << "variable\n";
    
    indent++;
    write_indent();
    os << "name = " << std::string(node->get_name().start, node->get_name().length);
    indent--;
}

void AstPrinter::visit_literal(Literal* node){
    write_indent();
    os << "literal " << std::string(node->get_value().start, node->get_value().length);
}

void AstPrinter::visit_string(String* node){
    write_indent();
    os << "string " << std::string(node->get_value().start, node->get_value().length);
}

void AstPrinter::visit_number(Number* node){
    write_indent();
    os << "number " << std::string(node->get_value().start, node->get_value().length);
}

void AstPrinter::write_indent() {
    static constexpr const char* indent_with = "  ";
    for (int i = 0; i < indent; ++i) os << indent_with;
}
