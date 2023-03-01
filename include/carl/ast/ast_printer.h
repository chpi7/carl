#ifndef CARL_AST_PRINTER_H
#define CARL_AST_PRINTER_H

#include "carl/ast/ast.h"

namespace carl {
class AstPrinter : public AstNodeVisitor {
   private:
    std::ostream& os;
    int indent;

   public:
    AstPrinter(std::ostream& os) : os(os), indent(0) {};
    void print(AstNode* node);
    void visit_invalid(Invalid* invalid);
    void visit_block(Block* block);
    void visit_fndecl(FnDecl* fndecl);
    void visit_formalparam(FormalParam* formalparam);
    void visit_returnstmt(ReturnStmt* returnstmt);
    void visit_whilestmt(WhileStmt* whilestmt);
    void visit_letstmt(LetStmt* letstmt);
    void visit_exprstmt(ExprStmt* exprstmt);
    void visit_binary(Binary* node);
    void visit_assignment(Assignment* node);
    void visit_unary(Unary* node);
    void visit_variable(Variable* node);
    void visit_literal(Literal* node);
    void visit_string(String* node);
    void visit_number(Number* node);
private:
    void write_indent();
};
}

#endif