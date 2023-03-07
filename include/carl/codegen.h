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
    void generate(std::vector<std::shared_ptr<AstNode>>& nodes);
    Chunk* get_chunk();
    std::unique_ptr<Chunk> take_chunk();

   private:
    uint64_t hash_string(const char* s, int length);

    void visit_invalid(Invalid* invalid);
    void visit_binary(Binary* binary);
    void visit_assignment(Assignment* binary);
    void visit_unary(Unary* unary);
    void visit_variable(Variable* variable);
    void visit_literal(Literal* literal);
    void visit_string(String* string);
    void visit_number(Number* number);
    void visit_exprstmt(ExprStmt* exprstmt);
    void visit_letdecl(LetDecl* letdecl);
    void visit_block(Block* letdecl);
    void visit_whilestmt(WhileStmt* letdecl);
};
}  // namespace carl

#endif