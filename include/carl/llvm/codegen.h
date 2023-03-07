#ifndef carl_llvm_codegen_h
#define carl_llvm_codegen_h

#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"

#include <memory>
#include <map>

#include "carl/ast/ast.h"

namespace carl {

class LLVMCodeGenerator : public AstNodeVisitor {
   private:
    llvm::Value* result;
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;
    std::map<std::string, llvm::Value*> names_values;

   public:
    LLVMCodeGenerator();
    // TODO: implement these, then load into the JIT
    void generate_eval();
    void generate_decl();

    llvm::Value* do_visit(AstNode* node) {
        node->accept(this);
        return result;
    }
    void visit_invalid(Invalid* invalid);
    void visit_exprstmt(ExprStmt* exprstmt);
    void visit_returnstmt(ReturnStmt* returnstmt);
    void visit_whilestmt(WhileStmt* whilestmt);
    void visit_block(Block* block);
    void visit_fndecl(FnDecl* fndecl);
    void visit_formalparam(FormalParam* formalparam);
    void visit_letdecl(LetDecl* letdecl);
    void visit_assignment(Assignment* assignment);
    void visit_binary(Binary* binary);
    void visit_unary(Unary* unary);
    void visit_variable(Variable* variable);
    void visit_literal(Literal* literal);
    void visit_string(String* string);
    void visit_number(Number* number);
};
}  // namespace carl

#endif