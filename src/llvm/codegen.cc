#include "carl/llvm/codegen.h"

#include <functional>

#include "carl/ast/types.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

using namespace carl;

static llvm::Value* getConstantInt(llvm::LLVMContext& ctx, uint64_t value) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), value, false);
}

static llvm::Function* getMallocFunction(llvm::Module& mod, llvm::LLVMContext& ctx) {
    auto* f = mod.getFunction("__malloc");
    if (f) return f;

    auto void_ptr_type = llvm::PointerType::getInt64PtrTy(ctx);
    auto size_t_type = llvm::PointerType::getInt64PtrTy(ctx);
    auto malloc_type = llvm::FunctionType::get(void_ptr_type, size_t_type, false);

    return llvm::Function::Create(malloc_type, llvm::Function::ExternalLinkage, "__malloc", mod);
}

static llvm::Function* getStringConcatFunction(llvm::Module& mod, llvm::LLVMContext& ctx) {
    auto* f = mod.getFunction("__string_concat");
    if (f) return f;

    auto ptr_type = llvm::PointerType::get(ctx, 0);
    auto fn_type =
        llvm::FunctionType::get(ptr_type, {ptr_type, ptr_type}, false);
    return llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage,
                                  "__string_concat", mod);
}

static llvm::Function* getDebugFunction(llvm::Module& mod, llvm::LLVMContext& ctx) {
    auto* f = mod.getFunction("__debug");
    if (f) return f;

    auto void_ptr_type = llvm::PointerType::getVoidTy(ctx);
    auto size_t_type = llvm::PointerType::getInt64PtrTy(ctx);
    auto malloc_type = llvm::FunctionType::get(void_ptr_type, size_t_type, false);

    return llvm::Function::Create(malloc_type, llvm::Function::ExternalLinkage, "__debug", mod);
}

static llvm::Function* getFunction(llvm::Module& mod, llvm::LLVMContext& ctx,
                                   FnDecl* decl = nullptr, bool create = false,
                                   const char* suffix = nullptr) {
    std::string fname = decl->get_sname();
    if (suffix) fname += suffix;

    llvm::Function* f = mod.getFunction(fname);
    if (f) return f;
    if (!create || decl == nullptr) return nullptr;

    if (decl->get_type()->get_base_type() != types::BaseType::FN) {
        std::cerr << "Can't get function type of non fn base type."
                  << std::endl;
        return nullptr;
    }
    auto carl_fn_type = std::static_pointer_cast<types::Fn>(decl->get_type());
    auto llvm_fn_type =
        static_cast<llvm::FunctionType*>(carl_fn_type->get_llvm_fn_type(ctx));
    f = llvm::Function::Create(
        llvm_fn_type, llvm::Function::ExternalLinkage, fname, mod);

    int arg_idx = 0;
    for (auto& arg : decl->get_formals()) {
        f->getArg(arg_idx++)->setName(std::string(arg->get_name()));
    }

    return f;
}

static llvm::Function* getFreeFunction(llvm::Module& mod, llvm::LLVMContext& ctx) {
    auto* f = mod.getFunction("__free");
    if (f) return f;

    auto void_type = llvm::Type::getVoidTy(ctx);
    auto void_ptr_type = llvm::PointerType::getInt64PtrTy(ctx);
    auto free_type = llvm::FunctionType::get(void_type, void_ptr_type, false);

    return llvm::Function::Create(free_type, llvm::Function::ExternalLinkage, "__free", mod);
}

static llvm::FunctionType* getFnTypeForCall(llvm::LLVMContext& ctx,
                                            Call* call) {
    auto ret_type = call->get_type()->get_llvm_rt_type(ctx);
    std::vector<llvm::Type*> fps;
    for (auto& param : call->get_arguments())
        fps.push_back(param->get_type()->get_llvm_rt_type(ctx));
    return llvm::FunctionType::get(ret_type, fps, false);
}

static llvm::AllocaInst* createAlloca(llvm::BasicBlock* bb, llvm::StringRef name, llvm::Type* type) {
    llvm::IRBuilder<> tmp_builder(bb, bb->begin());
    return tmp_builder.CreateAlloca(type, nullptr, name);
}

LLVMCodeGenerator::LLVMCodeGenerator() { 
    names = std::make_unique<Environment<TypedValue>>(nullptr);
    initialize();
}

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

static llvm::Function* createExtCall(llvm::LLVMContext& context, llvm::Module& mod) {
    auto voidType = llvm::Type::getVoidTy(context);
    llvm::FunctionType* voidFunType = llvm::FunctionType::get(voidType, false);
    llvm::Function* voidFun = llvm::Function::Create(voidFunType, llvm::Function::ExternalLinkage, "set_val", mod);
    return voidFun;
}

void LLVMCodeGenerator::generate_dummy() {
    start_wrapper_function();    
    auto f = createExtCall(*context, *module);
    builder->CreateCall(f, llvm::None, "set_val_call");

    auto intType = llvm::Type::getInt32Ty(*context);
    llvm::FunctionType* incType = llvm::FunctionType::get(intType, intType, false);
    llvm::Function* incFun = llvm::Function::Create(incType, llvm::Function::ExternalLinkage, "inc_val", *module);

    auto intConst = llvm::ConstantInt::getSigned(intType, 2);
    builder->CreateCall(incFun, intConst, "inc_call");

    end_wrapper_function();
}

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
    fpm.add(llvm::createInstructionCombiningPass());
    fpm.add(llvm::createReassociatePass());
    fpm.add(llvm::createGVNPass());
    fpm.add(llvm::createCFGSimplificationPass());
    fpm.doInitialization();
    fpm.run(*wrapper_fn);
}

llvm::orc::ThreadSafeModule LLVMCodeGenerator::take_module(bool print_module) {
    if (print_module) module->print(llvm::outs(), nullptr);
    return llvm::orc::ThreadSafeModule(std::move(module), std::move(context));
}

void LLVMCodeGenerator::visit_exprstmt(ExprStmt* exprstmt) {
    // Do nothing with the result --> statement.
    do_visit(exprstmt->get_expr());
}

void LLVMCodeGenerator::visit_returnstmt(ReturnStmt* returnstmt) {
    auto ret_type = returnstmt->get_expr()->get_type()->get_llvm_rt_type(*context);
    if (ret_type->isVoidTy()) {
        result = builder->CreateRetVoid();
    } else {
        auto ret_val = do_visit(returnstmt->get_expr());
        result = builder->CreateRet(ret_val);
    }
}

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

void LLVMCodeGenerator::visit_fndecl(FnDecl* fndecl) {
    // this doesnt do anything atm
    // for (auto& arg : fndecl->get_formals()) do_visit(arg);

    // 1. generate code for the actual function
    auto f = getFunction(*module, *context, fndecl, true, "_impl");
    if (!fndecl->get_is_extern()) {
        llvm::BasicBlock* current_bb = builder->GetInsertBlock();
        llvm::BasicBlock* body = llvm::BasicBlock::Create(*context, "entry", f);
        builder->SetInsertPoint(body);
        names->push();
        int arg_idx = 0;
        for (auto& arg : fndecl->get_formals()) {
            std::string name = arg->get_name();
            llvm::Value* v = f->getArg(arg_idx++);
            TypedValue aa = createAlloca(body, name, v->getType());
            builder->CreateStore(v, aa.value);
            names->set_variable(name, aa);
        }
        do_visit(fndecl->get_body());
        builder->CreateRetVoid();
        names->pop();
        builder->SetInsertPoint(current_bb);
    }

    // 2. create the runtime heap obj so we can easily pass around functions
    std::string fname = fndecl->get_sname();
    auto runtime_type = fndecl->get_type()->get_llvm_rt_type(*context);
    auto runtime_type_sz = llvm::ConstantExpr::getSizeOf(runtime_type);
    auto malloc = getMallocFunction(*module, *context);
    llvm::Value* addr = builder->CreateCall(malloc, runtime_type_sz, fname + "_wrapper_ptr");
    auto zero = getConstantInt(*context, 0);
    llvm::Value* fn_ptr_gep = builder->CreateGEP(
        runtime_type, addr,
        {zero, zero});
    builder->CreateStore(f, fn_ptr_gep);

    auto glob_name = fname;
    module->getOrInsertGlobal(glob_name, runtime_type->getPointerTo());
    auto global = module->getNamedGlobal(glob_name);
    global->setLinkage(llvm::GlobalValue::CommonLinkage);
    global->setAlignment(llvm::Align(8));
    global->setConstant(false);
    global->setInitializer(builder->getInt64(0));
    builder->CreateStore(addr, global);
    names->set_variable(fname, global);

    result = nullptr;
}

void LLVMCodeGenerator::visit_formalparam(FormalParam* formalparam) {
    // nothing to do here atm.
}

void LLVMCodeGenerator::visit_letdecl(LetDecl* letdecl) {
    std::string name(letdecl->get_name());
    auto initializer = letdecl->get_initializer();
    auto init_type = initializer->get_type();

    llvm::Value* initial_value = do_visit(initializer);
    auto alloca = createAlloca(builder->GetInsertBlock(), name, initial_value->getType());
    builder->CreateStore(initial_value, alloca);

    names->set_variable(name, alloca);
    result = nullptr;
}

void LLVMCodeGenerator::visit_assignment(Assignment* assignment) {
    // right now the parser only allows assignment to variables.
    auto target_variable =
        std::reinterpret_pointer_cast<Variable>(assignment->get_target());
    auto name = std::string(target_variable->get_name().start,
                            target_variable->get_name().length);

    if (!names->has_variable(name)) {
        log_error("named value " + name + " not found.");
        return;
    }
    auto variable = names->get_variable(name);

    auto value = do_visit(assignment->get_expr());

    result = builder->CreateStore(value, variable.value);
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
    
    auto lhs_type = binary->get_lhs()->get_type()->get_base_type();
    auto rhs_type = binary->get_rhs()->get_type()->get_base_type();
    auto is_string = lhs_type == rhs_type && lhs_type == types::BaseType::STRING;

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
                result = builder->CreateAdd(lhs, rhs, "add");
                break;
            case TOKEN_MINUS:
                result = builder->CreateSub(lhs, rhs, "sub");
                break;
            case TOKEN_STAR:
                result = builder->CreateMul(lhs, rhs, "mul");
                break;
            case TOKEN_SLASH:
                result = builder->CreateSDiv(lhs, rhs, "div");
                break;
            case TOKEN_PERC:
                result = builder->CreateSRem(lhs, rhs, "rem");
                break;
            case TOKEN_EQUAL_EQUAL:
                result = builder->CreateICmpEQ(lhs, rhs, "cmp");
                break;
            case TOKEN_LESS:
                result = builder->CreateICmpSLT(lhs, rhs, "cmp");
                break;
            case TOKEN_LESS_EQUAL:
                result = builder->CreateICmpSLE(lhs, rhs, "cmp");
                break;
            case TOKEN_GREATER:
                result = builder->CreateICmpSGT(lhs, rhs, "cmp");
                break;
            case TOKEN_GREATER_EQUAL:
                result = builder->CreateICmpSGE(lhs, rhs, "cmp");
                break;
            default:
                log_error("unsupported int binop found");
                break;
        }
    } else if (is_string && op_token == TOKEN_PLUS) {
        auto* concat_fn = getStringConcatFunction(*module, *context);
        result = builder->CreateCall(concat_fn, {lhs, rhs}, "concat_result");
    } else {
        // unsupported.
        result = nullptr;
    }
}

void LLVMCodeGenerator::visit_unary(Unary* unary) {
    auto op = unary->get_op().type;
    auto operand = do_visit(unary->get_operand());
    switch (op) {
        case TOKEN_MINUS: {
            result = builder->CreateNeg(operand);
            break;
        }
        case TOKEN_BANG: {
            result = builder->CreateNot(operand);
            break;
        }
        default: log_error("unsupported binop " + std::string(unary->get_op()));
    }
}

void LLVMCodeGenerator::visit_variable(Variable* variable) {
    std::string name = variable->get_name();
    if (names->has_variable(name)) {
        auto v = names->get_variable(name);
        result = builder->CreateLoad(v.type, v.value, name.c_str());
    } else {
        log_error("named value " + name + " not found.");
        result = nullptr;
    }
}

void LLVMCodeGenerator::visit_literal(Literal* literal) {
    if (literal->get_type()->get_base_type() == types::BaseType::BOOL) {
        bool val = std::string(literal->get_value()) == "true";
        result = llvm::ConstantInt::getBool(literal->get_type()->get_llvm_rt_type(*context), val);
    } else {
        log_error("unsupported base type for literal");
    }
}

void LLVMCodeGenerator::visit_string(String* string) {
    const char* start = string->get_value().start + 1;
    int len = string->get_value().length - 2;
    auto x = llvm::StringRef(start, len);
    auto const_str = llvm::ConstantDataArray::getString(*context, x);

    auto heap_type = string->get_type()->get_llvm_rt_type(*context);
    auto malloc = getMallocFunction(*module, *context);
    auto str_wrapper = builder->CreateCall(malloc, llvm::ConstantExpr::getSizeOf(heap_type));
    auto str_len = llvm::ConstantExpr::getSizeOf(const_str->getType());
    auto str_addr = builder->CreateCall(malloc, str_len);
    
    auto zero = getConstantInt(*context, 0);
    auto one = getConstantInt(*context, 1);

    auto wr_st_pt = builder->CreateGEP(heap_type, str_wrapper, {zero, one}, "str_data");
    auto wr_ln_pt = builder->CreateGEP(heap_type, str_wrapper, {zero, zero}, "str_len");
    builder->CreateStore(str_addr, wr_st_pt);
    builder->CreateStore(str_len, wr_ln_pt);
    builder->CreateStore(const_str, str_addr);

    result = str_wrapper;
}

void LLVMCodeGenerator::visit_number(Number* number) {
    auto num_type = number->get_type()->get_llvm_rt_type(*context);
    if (number->get_type()->get_base_type() == types::BaseType::FLOAT) {
        auto num = (double)atoi(number->get_value().start);
        result = llvm::ConstantFP::get(num_type, num);
    } else if (number->get_type()->get_base_type() == types::BaseType::INT) {
        auto num = atoi(number->get_value().start);
        result = llvm::ConstantInt::getSigned(num_type, num);
    }
}

void LLVMCodeGenerator::visit_call(Call* call) {
    // we assume the parser + typechecker have made sure this is a valid call.
    // --> there is always smth to call

    std::string fname = call->get_fname();
    TypedValue wrapper_var = names->get_variable(fname);
    llvm::Value* fn_heap_obj_ptr = builder->CreateLoad(wrapper_var.type, wrapper_var.value, "ld_fn_wrapper");
    auto zero = getConstantInt(*context, 0);
    llvm::Value* ptr_to_fn_addr = builder->CreateGEP(
        llvm::StructType::getTypeByName(*context, "__carl_fn"), fn_heap_obj_ptr,
        {zero, zero}, "ptr_to_fn_addr");
    
    // opaque ptrs so it doesnt matter what it actually is :)
    auto some_ptr = llvm::PointerType::get(*context, 0);
    llvm::Value* fn_addr = builder->CreateLoad(some_ptr, ptr_to_fn_addr, fname);

    std::vector<llvm::Value*> args;
    for (auto& arg : call->get_arguments()) {
        auto* arg_v = do_visit(arg);
        args.push_back(arg_v);
    }

    auto fn_type = getFnTypeForCall(*context, call);
    result = builder->CreateCall(fn_type, fn_addr, args, fname + "_res");
}