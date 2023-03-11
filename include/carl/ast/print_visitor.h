#ifndef CARL_PRINT_VISITOR_H
#define CARL_PRINT_VISITOR_H

#include "carl/ast/ast.h"

namespace carl {
class PrintAstNodeVisitor : public AstNodeVisitor {
   private:
    std::ostream& os;

   public:
    PrintAstNodeVisitor(std::ostream& os) : os(os){};
    void visit_block(Block* block);
    void visit_whilestmt(WhileStmt* whilestmt);
    void visit_letdecl(LetDecl* letdecl);
    void visit_exprstmt(ExprStmt* exprstmt);
    void visit_binary(Binary* node);
    void visit_assignment(Assignment* node);
    void visit_unary(Unary* node);
    void visit_variable(Variable* node);
    void visit_literal(Literal* node);
    void visit_string(String* node);
    void visit_number(Number* node);
};
}  // namespace carl
#endif