#include "carl/ast/ast_printer.h"

using namespace carl;

void AstPrinter::visit_block(Block* block) {
    write_indent();
    os << "Block" << "\n"; 
    indent++;
    write_indent();
    os << ".declarations\n";
    indent++;
    for (auto& elem : block->get_declarations()) {
        elem->accept(this);
    }
    indent--;
    indent--;
}

void AstPrinter::visit_type(Type* type) {
    write_indent();
    os << "Type" << "\n"; 
    indent++;
    write_indent();
    os << ".name = " << std::string(type->get_name().start, type->get_name().length) << "\n";
    indent--;
}

void AstPrinter::visit_formalparam(FormalParam* formalparam) {
    write_indent();
    os << "FormalParam [" << formalparam->get_type()->str() << "]\n"; 
    indent++;
    write_indent();
    os << ".name = " << std::string(formalparam->get_name().start, formalparam->get_name().length) << "\n";

    indent--;
}

void AstPrinter::visit_fndecl(FnDecl* fndecl) {
    write_indent();
    os << "FnDecl [" << fndecl->get_type()->str() << "]\n"; 
    indent++;
    write_indent();
    os << ".name = " << std::string(fndecl->get_name().start, fndecl->get_name().length) << "\n";
    write_indent();
    os << ".sname = " << fndecl->get_sname() << "\n";
    write_indent();
    os << ".formals\n";
    indent++;
    for (auto& elem : fndecl->get_formals()) {
        elem->accept(this);
    }
    indent--;
    write_indent();
    os << ".body\n";
    indent++;
    fndecl->get_body()->accept(this);
    indent--;

    write_indent();
    os << ".captures\n";
    indent++;
    for (auto& elem : fndecl->get_captures()) {
        elem->accept(this);
    }
    indent--;
    write_indent();
    os << ".is_extern = " << (fndecl->get_is_extern() ? std::string("true") : std::string("false")) << "\n";
    indent--;
}

void AstPrinter::visit_letdecl(LetDecl* letdecl) {
    write_indent();
    os << "LetDecl [" << letdecl->get_type()->str() << "]\n"; 
    indent++;
    write_indent();
    os << ".name = " << std::string(letdecl->get_name().start, letdecl->get_name().length) << "\n";
    write_indent();
    os << ".initializer\n";
    indent++;
    letdecl->get_initializer()->accept(this);
    indent--;

    indent--;
}

void AstPrinter::visit_adtstmt(AdtStmt* adtstmt) {
    write_indent();
    os << "AdtStmt [" << adtstmt->get_type()->str() << "]\n"; 
    indent++;

    indent--;
}

void AstPrinter::visit_exprstmt(ExprStmt* exprstmt) {
    write_indent();
    os << "ExprStmt" << "\n"; 
    indent++;
    write_indent();
    os << ".expr\n";
    indent++;
    exprstmt->get_expr()->accept(this);
    indent--;
    indent--;
}

void AstPrinter::visit_returnstmt(ReturnStmt* returnstmt) {
    write_indent();
    os << "ReturnStmt" << "\n"; 
    indent++;
    write_indent();
    os << ".expr\n";
    indent++;
    returnstmt->get_expr()->accept(this);
    indent--;
    indent--;
}

void AstPrinter::visit_whilestmt(WhileStmt* whilestmt) {
    write_indent();
    os << "WhileStmt" << "\n"; 
    indent++;
    write_indent();
    os << ".condition\n";
    indent++;
    whilestmt->get_condition()->accept(this);
    indent--;
    write_indent();
    os << ".body\n";
    indent++;
    whilestmt->get_body()->accept(this);
    indent--;
    indent--;
}

void AstPrinter::visit_assignment(Assignment* assignment) {
    write_indent();
    os << "Assignment [" << assignment->get_type()->str() << "]\n"; 
    indent++;
    write_indent();
    os << ".target\n";
    indent++;
    assignment->get_target()->accept(this);
    indent--;
    write_indent();
    os << ".expr\n";
    indent++;
    assignment->get_expr()->accept(this);
    indent--;
    indent--;
}

void AstPrinter::visit_binary(Binary* binary) {
    write_indent();
    os << "Binary [" << binary->get_type()->str() << "]\n"; 
    indent++;
    write_indent();
    os << ".op = " << std::string(binary->get_op().start, binary->get_op().length) << "\n";
    write_indent();
    os << ".lhs\n";
    indent++;
    binary->get_lhs()->accept(this);
    indent--;
    write_indent();
    os << ".rhs\n";
    indent++;
    binary->get_rhs()->accept(this);
    indent--;
    indent--;
}

void AstPrinter::visit_unary(Unary* unary) {
    write_indent();
    os << "Unary [" << unary->get_type()->str() << "]\n"; 
    indent++;
    write_indent();
    os << ".op = " << std::string(unary->get_op().start, unary->get_op().length) << "\n";
    write_indent();
    os << ".operand\n";
    indent++;
    unary->get_operand()->accept(this);
    indent--;
    indent--;
}

void AstPrinter::visit_variable(Variable* variable) {
    write_indent();
    os << "Variable [" << variable->get_type()->str() << "]\n"; 
    indent++;
    write_indent();
    os << ".name = " << std::string(variable->get_name().start, variable->get_name().length) << "\n";
    indent--;
}

void AstPrinter::visit_literal(Literal* literal) {
    write_indent();
    os << "Literal [" << literal->get_type()->str() << "]\n"; 
    indent++;
    write_indent();
    os << ".value = " << std::string(literal->get_value().start, literal->get_value().length) << "\n";
    indent--;
}

void AstPrinter::visit_string(String* string) {
    write_indent();
    os << "String [" << string->get_type()->str() << "]\n"; 
    indent++;
    write_indent();
    os << ".value = " << std::string(string->get_value().start, string->get_value().length) << "\n";
    indent--;
}

void AstPrinter::visit_number(Number* number) {
    write_indent();
    os << "Number [" << number->get_type()->str() << "]\n"; 
    indent++;
    write_indent();
    os << ".value = " << std::string(number->get_value().start, number->get_value().length) << "\n";
    indent--;
}

void AstPrinter::visit_call(Call* call) {
    write_indent();
    os << "Call [" << call->get_type()->str() << "]\n"; 
    indent++;
    write_indent();
    os << ".fname = " << std::string(call->get_fname().start, call->get_fname().length) << "\n";
    write_indent();
    os << ".arguments\n";
    indent++;
    for (auto& elem : call->get_arguments()) {
        elem->accept(this);
    }
    indent--;
    indent--;
}

void AstPrinter::visit_matcharm(MatchArm* matcharm) {
    write_indent();
    os << "MatchArm [" << matcharm->get_type()->str() << "]\n"; 
    indent++;
    write_indent();
    os << ".result\n";
    indent++;
    matcharm->get_result()->accept(this);
    indent--;
    indent--;
}

void AstPrinter::visit_match(Match* match) {
    write_indent();
    os << "Match [" << match->get_type()->str() << "]\n"; 
    indent++;
    write_indent();
    os << ".matchee\n";
    indent++;
    match->get_matchee()->accept(this);
    indent--;
    write_indent();
    os << ".arms\n";
    indent++;
    for (auto& elem : match->get_arms()) {
        elem->accept(this);
    }
    indent--;
    indent--;
}
