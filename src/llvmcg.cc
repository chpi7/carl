#include "carl/llvmcg.h"

using namespace carl;

LLVMCodeGenerator::LLVMCodeGenerator() {
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
}

void LLVMCodeGenerator::visit_invalid(Invalid* invalid) {}
void LLVMCodeGenerator::visit_exprstmt(ExprStmt* exprstmt) {}
void LLVMCodeGenerator::visit_returnstmt(ReturnStmt* returnstmt) {}
void LLVMCodeGenerator::visit_whilestmt(WhileStmt* whilestmt) {}
void LLVMCodeGenerator::visit_block(Block* block) {}
void LLVMCodeGenerator::visit_fndecl(FnDecl* fndecl) {}
void LLVMCodeGenerator::visit_formalparam(FormalParam* formalparam) {}
void LLVMCodeGenerator::visit_letstmt(LetStmt* letstmt) {}
void LLVMCodeGenerator::visit_assignment(Assignment* assignment) {}
void LLVMCodeGenerator::visit_binary(Binary* binary) {
    auto lhs = do_visit(binary->get_lhs().get());
    auto rhs = do_visit(binary->get_rhs().get());
    if (lhs->getType()->isDoubleTy() && rhs->getType()->isDoubleTy()) {
        result = builder->CreateFAdd(lhs, rhs, "faddtmp");
    } else if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
        result = builder->CreateAdd(lhs, rhs, "addtmp");
    }
}
void LLVMCodeGenerator::visit_unary(Unary* unary) {}
void LLVMCodeGenerator::visit_variable(Variable* variable) {}
void LLVMCodeGenerator::visit_literal(Literal* literal) {}
void LLVMCodeGenerator::visit_string(String* string) {}
void LLVMCodeGenerator::visit_number(Number* number) {
}