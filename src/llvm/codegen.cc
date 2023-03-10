#include "carl/llvm/codegen.h"

#include <functional>

#include "carl/ast/types.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

using namespace carl;

static llvm::Type* getIntType(llvm::LLVMContext& context) {
    return llvm::IntegerType::getInt64Ty(context);
}

LLVMCodeGenerator::LLVMCodeGenerator() { initialize(); }

void LLVMCodeGenerator::log_error(std::string s) {
    has_error = true;
    std::cerr << "Codegen error: " << s << std::endl;
}

void LLVMCodeGenerator::initialize() {
    has_error = false;
    if (context) {
        context.release();
        module.release();
        builder.release();
    }
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("some module", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
}

llvm::Function* LLVMCodeGenerator::start_wrapper_function() {
    auto wrapper_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(*context), false);
    auto wrapper = llvm::Function::Create(
        wrapper_type, llvm::Function::ExternalLinkage, "__main", *module);
    auto bb = llvm::BasicBlock::Create(*context, "entry", wrapper);
    builder->SetInsertPoint(bb);

    return wrapper;
}

void LLVMCodeGenerator::end_wrapper_function() { builder->CreateRetVoid(); }

void LLVMCodeGenerator::generate_eval(std::shared_ptr<Expression> expr) {
    initialize();

    auto wrapper_type =
        llvm::FunctionType::get(llvm::Type::getInt64Ty(*context), false);
    auto wrapper =
        llvm::Function::Create(wrapper_type, llvm::Function::ExternalLinkage,
                               "__expr_wrapper", *module);
    auto bb = llvm::BasicBlock::Create(*context, "entry", wrapper);
    builder->SetInsertPoint(bb);

    auto result = do_visit(expr);
    auto rtype = result->getType();
    if (!rtype->isIntegerTy()) {
        log_error("invalid expr ret type");
    }

    builder->CreateRet(result);
}

void LLVMCodeGenerator::generate(
    std::vector<std::shared_ptr<AstNode>> declarations) {
    initialize();
    auto wrapper_fn = start_wrapper_function();

    for (auto& decl : declarations) {
        do_visit(decl);
    }

    end_wrapper_function();

    llvm::legacy::FunctionPassManager fpm(module.get());
    // fpm.add(llvm::createInstructionCombiningPass());
    // fpm.add(llvm::createReassociatePass());
    // fpm.add(llvm::createGVNPass());
    // fpm.add(llvm::createCFGSimplificationPass());
    fpm.doInitialization();
    fpm.run(*wrapper_fn);
}

llvm::orc::ThreadSafeModule LLVMCodeGenerator::take_module() {
    module->print(llvm::outs(), nullptr);
    return llvm::orc::ThreadSafeModule(std::move(module), std::move(context));
}

void LLVMCodeGenerator::visit_exprstmt(ExprStmt* exprstmt) {
    // Do nothing with the result --> statement.
    do_visit(exprstmt->get_expr());
}

void LLVMCodeGenerator::visit_returnstmt(ReturnStmt* returnstmt) {}

void LLVMCodeGenerator::visit_whilestmt(WhileStmt* whilestmt) {
    auto parent_fn = builder->GetInsertBlock()->getParent();
    auto cond_block = llvm::BasicBlock::Create(*context, "while_cond", parent_fn);
    auto body_block = llvm::BasicBlock::Create(*context, "while_body", parent_fn);
    auto after_block = llvm::BasicBlock::Create(*context, "after_while", parent_fn);
    
    builder->CreateBr(cond_block);

    // condition
    builder->SetInsertPoint(cond_block);
    auto cond_expr = do_visit(whilestmt->get_condition()); 
    auto cond_type = cond_expr->getType();
    if (!(cond_type->isIntegerTy() && cond_type->getIntegerBitWidth() == 1)) {
        log_error("while condition should be int1 (bool) type");
        result = nullptr;
        return;
    }
    builder->CreateCondBr(cond_expr, body_block, after_block);

    // body
    builder->SetInsertPoint(body_block);
    do_visit(whilestmt->get_body());
    builder->CreateBr(cond_block);

    // after
    builder->SetInsertPoint(after_block);
}

void LLVMCodeGenerator::visit_block(Block* block) {
    for (auto& x : block->get_declarations()) {
        do_visit(x);
    }
}

void LLVMCodeGenerator::visit_fndecl(FnDecl* fndecl) {}

void LLVMCodeGenerator::visit_formalparam(FormalParam* formalparam) {}

void LLVMCodeGenerator::visit_letdecl(LetDecl* letdecl) {
    auto name =
        std::string(letdecl->get_name().start, letdecl->get_name().length);
    auto initial_value = do_visit(letdecl->get_initializer());

    auto inst =
        builder->CreateAlloca(getIntType(*context), nullptr, name.c_str());
    names_values.insert(std::make_pair(name, inst));
    builder->CreateStore(initial_value, inst);
}

void LLVMCodeGenerator::visit_assignment(Assignment* assignment) {
    // right now the parser only allows assignment to variables.
    auto target_variable =
        std::reinterpret_pointer_cast<Variable>(assignment->get_target());
    auto name = std::string(target_variable->get_name().start,
                            target_variable->get_name().length);

    if (!names_values.contains(name)) {
        log_error("named value " + name + " not found.");
        return;
    }
    auto target_alloca = names_values.at(name);

    auto value = do_visit(assignment->get_expr());

    result = builder->CreateStore(value, target_alloca);
}

void LLVMCodeGenerator::visit_binary(Binary* binary) {
    auto lhs = do_visit(binary->get_lhs());
    auto rhs = do_visit(binary->get_rhs());
    auto op_token = binary->get_op().type;

    // TODO cast if types are not equal here.

    auto is_float =
        lhs->getType()->isDoubleTy() && rhs->getType()->isDoubleTy();
    auto is_int =
        lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy();

    if (is_float) {
        switch (op_token) {
            case TOKEN_PLUS:
                result = builder->CreateFAdd(lhs, rhs, "fadd");
                break;
            case TOKEN_MINUS:
                result = builder->CreateFSub(lhs, rhs, "fsub");
                break;
            case TOKEN_STAR:
                result = builder->CreateFMul(lhs, rhs, "fmul");
                break;
            case TOKEN_SLASH:
                result = builder->CreateFDiv(lhs, rhs, "fdiv");
                break;
            case TOKEN_EQUAL_EQUAL:
                result = builder->CreateFCmpUEQ(lhs, rhs, "fcmp");
                break;
            case TOKEN_LESS:
                result = builder->CreateFCmpULT(lhs, rhs, "fcmp");
                break;
            case TOKEN_LESS_EQUAL:
                result = builder->CreateFCmpULE(lhs, rhs, "fcmp");
                break;
            case TOKEN_GREATER:
                result = builder->CreateFCmpUGT(lhs, rhs, "fcmp");
                break;
            case TOKEN_GREATER_EQUAL:
                result = builder->CreateFCmpUGE(lhs, rhs, "fcmp");
                break;
            default:
                log_error("unsupported float binop found");
                break;
        }
    } else if (is_int) {
        // Always choose signed operations.
        switch (op_token) {
            case TOKEN_PLUS:
                result = builder->CreateAdd(lhs, rhs, "fadd");
                break;
            case TOKEN_MINUS:
                result = builder->CreateSub(lhs, rhs, "fsub");
                break;
            case TOKEN_STAR:
                result = builder->CreateMul(lhs, rhs, "fmul");
                break;
            case TOKEN_SLASH:
                result = builder->CreateSDiv(lhs, rhs, "fdiv");
                break;
            case TOKEN_PERC:
                result = builder->CreateSRem(lhs, rhs, "frem");
                break;
            case TOKEN_EQUAL_EQUAL:
                result = builder->CreateICmpEQ(lhs, rhs, "fcmp");
                break;
            case TOKEN_LESS:
                result = builder->CreateICmpSLT(lhs, rhs, "fcmp");
                break;
            case TOKEN_LESS_EQUAL:
                result = builder->CreateICmpSLE(lhs, rhs, "fcmp");
                break;
            case TOKEN_GREATER:
                result = builder->CreateICmpSGT(lhs, rhs, "fcmp");
                break;
            case TOKEN_GREATER_EQUAL:
                result = builder->CreateICmpSGE(lhs, rhs, "fcmp");
                break;
            default:
                log_error("unsupported int binop found");
                break;
        }
    } else {
        // unsupported.
        result = nullptr;
    }
}

void LLVMCodeGenerator::visit_unary(Unary* unary) {}

void LLVMCodeGenerator::visit_variable(Variable* variable) {
    auto name =
        std::string(variable->get_name().start, variable->get_name().length);
    if (!names_values.contains(name)) {
        log_error("named value " + name + " not found.");
        result = nullptr;
        return;
    }
    auto alloca = names_values.at(name);
    result =
        builder->CreateLoad(alloca->getAllocatedType(), alloca, name.c_str());
}

void LLVMCodeGenerator::visit_literal(Literal* literal) {}
void LLVMCodeGenerator::visit_string(String* string) {}
void LLVMCodeGenerator::visit_number(Number* number) {
    if (number->get_type()->get_base_type() == types::BaseType::FLOAT) {
        auto num = (double)atoi(number->get_value().start);
        result = llvm::ConstantFP::get(*context, llvm::APFloat(num));
    } else if (number->get_type()->get_base_type() == types::BaseType::INT) {
        auto num = atoi(number->get_value().start);
        result = llvm::ConstantInt::getSigned(getIntType(*context), num);
    }
}