#ifndef carl_llvm_codegen_h
#define carl_llvm_codegen_h

#include <map>
#include <memory>

#include "carl/ast/ast.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"

namespace carl {

class LLVMCodeGenerator : public AstNodeVisitor {
   private:
    bool has_error;
    llvm::Value* result;
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;
    std::map<std::string, llvm::AllocaInst*> names_values;

   private:
    void log_error(std::string s);
    llvm::Function* start_wrapper_function();
    void end_wrapper_function();

   public:
    LLVMCodeGenerator();
    void initialize();
    llvm::orc::ThreadSafeModule take_module(bool print_module = false);
    void generate_dummy();
    void generate_eval(std::shared_ptr<Expression> expr);
    void generate(std::vector<std::shared_ptr<AstNode>> declarations);

    llvm::Value* do_visit(std::shared_ptr<AstNode> node) {
        node->accept(this);
        return result;
    }
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
    void visit_call(Call* call);
};
}  // namespace carl

#endif