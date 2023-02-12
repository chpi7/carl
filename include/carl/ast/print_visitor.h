#include "carl/ast/ast.h"

namespace carl {
class PrintAstNodeVisitor : public AstNodeVisitor {
   private:
    std::ostream& os;

   public:
    PrintAstNodeVisitor(std::ostream& os) : os(os){};
    void visit_binary(Binary* node);
    void visit_unary(Unary* node);
    void visit_variable(Variable* node);
    void visit_literal(Literal* node);
    void visit_string(String* node);
    void visit_number(Number* node);
};
}  // namespace carl