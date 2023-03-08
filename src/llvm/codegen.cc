#include "carl/llvm/codegen.h"

#include <functional>

#include "carl/ast/types.h"

using namespace carl;

LLVMCodeGenerator::LLVMCodeGenerator() {
    initialize();
}

void LLVMCodeGenerator::initialize() {
    if (context) {
        context.release();
        module.release();
        builder.release();
    }
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("some module", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
}

void LLVMCodeGenerator::create_dummy_function() {
    initialize();

    auto main_t = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), false);
    auto main = llvm::Function::Create(main_t, llvm::Function::ExternalLinkage, "__main", *module);

    auto bb = llvm::BasicBlock::Create(*context, "entry", main); 
    builder->SetInsertPoint(bb);

    auto result = llvm::ConstantInt::getSigned(llvm::IntegerType::getInt32Ty(*context), 1337);
    builder->CreateRet(result);
}

llvm::orc::ThreadSafeModule LLVMCodeGenerator::take_module() {
    module->print(llvm::outs(), nullptr);

    auto tsm = llvm::orc::ThreadSafeModule(std::move(module), std::move(context));
    return tsm;
}


void LLVMCodeGenerator::visit_invalid(Invalid* invalid) {}
void LLVMCodeGenerator::visit_exprstmt(ExprStmt* exprstmt) {
    result = do_visit(exprstmt->get_expr().get());
}
void LLVMCodeGenerator::visit_returnstmt(ReturnStmt* returnstmt) {}
void LLVMCodeGenerator::visit_whilestmt(WhileStmt* whilestmt) {}
void LLVMCodeGenerator::visit_block(Block* block) {}
void LLVMCodeGenerator::visit_fndecl(FnDecl* fndecl) {}
void LLVMCodeGenerator::visit_formalparam(FormalParam* formalparam) {}
void LLVMCodeGenerator::visit_letdecl(LetDecl* letdecl) {}
void LLVMCodeGenerator::visit_assignment(Assignment* assignment) {}
void LLVMCodeGenerator::visit_binary(Binary* binary) {
    auto lhs = do_visit(binary->get_lhs().get());
    auto rhs = do_visit(binary->get_rhs().get());
    auto op_token = binary->get_op().type;

    // TODO cast if types are not equal here.

    auto is_float =
        lhs->getType()->isDoubleTy() && rhs->getType()->isDoubleTy();
    auto is_int =
        lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy();

    if (is_float) {
        switch (op_token) {
            case TOKEN_PLUS: result = builder->CreateFAdd(lhs, rhs, "fadd"); break;
            case TOKEN_MINUS: result = builder->CreateFSub(lhs, rhs, "fsub"); break;
            case TOKEN_STAR: result = builder->CreateFMul(lhs, rhs, "fmul"); break;
            case TOKEN_SLASH: result = builder->CreateFDiv(lhs, rhs, "fdiv"); break;
            case TOKEN_EQUAL_EQUAL: result = builder->CreateFCmpUEQ(lhs, rhs, "fcmp"); break;
            case TOKEN_LESS: result = builder->CreateFCmpULT(lhs, rhs, "fcmp"); break;
            case TOKEN_LESS_EQUAL: result = builder->CreateFCmpULE(lhs, rhs, "fcmp"); break;
            case TOKEN_GREATER: result = builder->CreateFCmpUGT(lhs, rhs, "fcmp"); break;
            case TOKEN_GREATER_EQUAL: result = builder->CreateFCmpUGE(lhs, rhs, "fcmp"); break;
            default: std::cerr << "unsupported float binop found" << std::endl; break;
        }
    } else if (is_int) {
        // Always choose signed operations.
        switch (op_token) {
            case TOKEN_PLUS: result = builder->CreateAdd(lhs, rhs, "fadd"); break;
            case TOKEN_MINUS: result = builder->CreateSub(lhs, rhs, "fsub"); break;
            case TOKEN_STAR: result = builder->CreateMul(lhs, rhs, "fmul"); break;
            case TOKEN_SLASH: result = builder->CreateSDiv(lhs, rhs, "fdiv"); break;
            case TOKEN_PERC: result = builder->CreateSRem(lhs, rhs, "frem"); break;
            case TOKEN_EQUAL_EQUAL: result = builder->CreateICmpEQ(lhs, rhs, "fcmp"); break;
            case TOKEN_LESS: result = builder->CreateICmpSLT(lhs, rhs, "fcmp"); break;
            case TOKEN_LESS_EQUAL: result = builder->CreateICmpSLE(lhs, rhs, "fcmp"); break;
            case TOKEN_GREATER: result = builder->CreateICmpSGT(lhs, rhs, "fcmp"); break;
            case TOKEN_GREATER_EQUAL: result = builder->CreateICmpSGE(lhs, rhs, "fcmp"); break;
            default: std::cerr << "unsupported int binop found" << std::endl; break;
        }
    } else {
        // unsupported.
        result = nullptr;
    }
}

void LLVMCodeGenerator::visit_unary(Unary* unary) {}
void LLVMCodeGenerator::visit_variable(Variable* variable) {}
void LLVMCodeGenerator::visit_literal(Literal* literal) {}
void LLVMCodeGenerator::visit_string(String* string) {}
void LLVMCodeGenerator::visit_number(Number* number) {
    if (number->get_type()->get_base_type() == types::BaseType::FLOAT) {
        auto num = (double)atoi(number->get_value().start);
        result = llvm::ConstantFP::get(*context, llvm::APFloat(num));
    } else if (number->get_type()->get_base_type() == types::BaseType::INT) {
        auto num = atoi(number->get_value().start);
        result = llvm::ConstantInt::getSigned(llvm::IntegerType::getInt64Ty(*context), num);
    }
}