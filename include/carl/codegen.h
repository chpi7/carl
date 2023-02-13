#ifndef carl_codegen_h
#define carl_codegen_h

#include "carl/ast/ast.h"
#include "carl/vm/chunk.h"

namespace carl {

class CodeGenerator : public AstNodeVisitor {
   private:
    std::unique_ptr<Chunk> chunk;

   public:
    CodeGenerator();
    void generate(std::shared_ptr<AstNode> node);
    Chunk* get_chunk();
    std::unique_ptr<Chunk> take_chunk();

   private:
    void visit_invalid(Invalid* invalid);
    void visit_binary(Binary* binary);
    void visit_unary(Unary* unary);
    void visit_variable(Variable* variable);
    void visit_literal(Literal* literal);
    void visit_string(String* string);
    void visit_number(Number* number);
    void visit_exprstmt(ExprStmt* exprstmt);
    void visit_letstmt(LetStmt* letstmt);
};
}  // namespace carl

#endif