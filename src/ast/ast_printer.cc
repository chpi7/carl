#include "carl/ast/ast_printer.h"

using namespace carl;

void AstPrinter::visit_invalid(Invalid* invalid) {
    write_indent();
    os << "Invalid" << "\n";
    indent++;

    indent--;
}

void AstPrinter::visit_type(Type* type) {
    write_indent();
    os << "Type" << "\n";
    indent++;
    write_indent();
    os << "name = " << std::string(type->get_name().start, type->get_name().length) << "\n";
    indent--;
}

void AstPrinter::visit_formalparam(FormalParam* formalparam) {
    write_indent();
    os << "FormalParam" << "\n";
    indent++;
    write_indent();
    os << "name = " << std::string(formalparam->get_name().start, formalparam->get_name().length) << "\n";

    indent--;
}

void AstPrinter::visit_fndecl(FnDecl* fndecl) {
    write_indent();
    os << "FnDecl" << "\n";
    indent++;
    write_indent();
    os << "name = " << std::string(fndecl->get_name().start, fndecl->get_name().length) << "\n";
    write_indent();
    os << "formals\n";
    indent++;
    for (auto& elem : fndecl->get_formals()) {
        elem->accept(this);
    }
    indent--;

    indent--;
}

void AstPrinter::visit_letdecl(LetDecl* letdecl) {
    write_indent();
    os << "LetDecl" << "\n";
    indent++;
    write_indent();
    os << "name = " << std::string(letdecl->get_name().start, letdecl->get_name().length) << "\n";

    indent--;
}

void AstPrinter::visit_exprstmt(ExprStmt* exprstmt) {
    write_indent();
    os << "ExprStmt" << "\n";
    indent++;

    indent--;
}

void AstPrinter::visit_returnstmt(ReturnStmt* returnstmt) {
    write_indent();
    os << "ReturnStmt" << "\n";
    indent++;

    indent--;
}

void AstPrinter::visit_whilestmt(WhileStmt* whilestmt) {
    write_indent();
    os << "WhileStmt" << "\n";
    indent++;


    indent--;
}

void AstPrinter::visit_block(Block* block) {
    write_indent();
    os << "Block" << "\n";
    indent++;
    write_indent();
    os << "declarations\n";
    indent++;
    for (auto& elem : block->get_declarations()) {
        elem->accept(this);
    }
    indent--;
    indent--;
}

void AstPrinter::visit_assignment(Assignment* assignment) {
    write_indent();
    os << "Assignment" << "\n";
    indent++;


    indent--;
}

void AstPrinter::visit_binary(Binary* binary) {
    write_indent();
    os << "Binary" << "\n";
    indent++;
    write_indent();
    os << "op = " << std::string(binary->get_op().start, binary->get_op().length) << "\n";


    indent--;
}

void AstPrinter::visit_unary(Unary* unary) {
    write_indent();
    os << "Unary" << "\n";
    indent++;
    write_indent();
    os << "op = " << std::string(unary->get_op().start, unary->get_op().length) << "\n";

    indent--;
}

void AstPrinter::visit_variable(Variable* variable) {
    write_indent();
    os << "Variable" << "\n";
    indent++;
    write_indent();
    os << "name = " << std::string(variable->get_name().start, variable->get_name().length) << "\n";
    indent--;
}

void AstPrinter::visit_literal(Literal* literal) {
    write_indent();
    os << "Literal" << "\n";
    indent++;
    write_indent();
    os << "value = " << std::string(literal->get_value().start, literal->get_value().length) << "\n";
    indent--;
}

void AstPrinter::visit_string(String* string) {
    write_indent();
    os << "String" << "\n";
    indent++;
    write_indent();
    os << "value = " << std::string(string->get_value().start, string->get_value().length) << "\n";
    indent--;
}

void AstPrinter::visit_number(Number* number) {
    write_indent();
    os << "Number" << "\n";
    indent++;
    write_indent();
    os << "value = " << std::string(number->get_value().start, number->get_value().length) << "\n";

    indent--;
}

void AstPrinter::visit_call(Call* call) {
    write_indent();
    os << "Call" << "\n";
    indent++;
    write_indent();
    os << "fname = " << std::string(call->get_fname().start, call->get_fname().length) << "\n";
    write_indent();
    os << "arguments\n";
    indent++;
    for (auto& elem : call->get_arguments()) {
        elem->accept(this);
    }
    indent--;
    indent--;
}
