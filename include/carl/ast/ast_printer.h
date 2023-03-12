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
    void print(AstNode* node) {
        indent = 0;
        node->accept(this);
        os << "\n";
    }
    void visit_type(Type* type);
    void visit_formalparam(FormalParam* formalparam);
    void visit_fndecl(FnDecl* fndecl);
    void visit_letdecl(LetDecl* letdecl);
    void visit_exprstmt(ExprStmt* exprstmt);
    void visit_returnstmt(ReturnStmt* returnstmt);
    void visit_whilestmt(WhileStmt* whilestmt);
    void visit_block(Block* block);
    void visit_assignment(Assignment* assignment);
    void visit_binary(Binary* binary);
    void visit_unary(Unary* unary);
    void visit_variable(Variable* variable);
    void visit_literal(Literal* literal);
    void visit_string(String* string);
    void visit_number(Number* number);
    void visit_call(Call* call);
    void visit_partialapp(PartialApp* partialapp);
private:
    void write_indent() {
        static constexpr const char* indent_with = "  ";
        for (int i = 0; i < indent; ++i) os << indent_with;
    }
};
}
#endif
    