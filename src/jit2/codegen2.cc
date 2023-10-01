#include "carl/jit2/codegen2.h"

#include "carl/jit2/runtime_types.h"
#include "carl/jit2/runtime_types_llvm.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

using namespace carl;

Codegen2::Codegen2() {}

void Codegen2::init(std::string module_name) {
    has_error = false;
    if (context) {
        context.release();
        module.release();
        builder.release();
    }
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>(module_name, *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
}

Codegen2Module Codegen2::generate(std::vector<std::shared_ptr<AstNode>> decls) {
    /* Init main wrapper function */
    bool return_last_result = false;
    llvm::Type* ret_type = llvm::Type::getVoidTy(*context);
    llvm::Value* last_result = nullptr;
    if (decls.back()->get_node_type() == AstNodeType::ExprStmt) {
        return_last_result = true;
        auto expr =
            std::dynamic_pointer_cast<ExprStmt>(decls.back())->get_expr();
        ret_type = runtime_type_llvm_get__from_BaseType(
            expr->get_type()->get_base_type(), *context);
    }
    llvm::Function* main = start_function("__carl_main", ret_type);

    /* Generate code for everything */
    for (size_t i = 0; i < decls.size(); ++i) {
        const auto& declaration = decls.at(i);
        bool is_last = i == decls.size() - 1;

        if (is_last && return_last_result) {
            /* We checked above that this cast is okay */
            auto expr =
                std::dynamic_pointer_cast<ExprStmt>(decls.back())->get_expr();
            last_result = do_visit(expr);
        } else {
            do_visit(declaration);
        }
    }

    /* Create return for main function if necessary */
    if (return_last_result) {
        builder->CreateRet(last_result);
    } else {
        builder->CreateRetVoid();
    }

    module->print(llvm::outs(), nullptr);

    /* Optimize and return module */
    llvm::legacy::FunctionPassManager fpm(module.get());
    fpm.add(llvm::createInstructionCombiningPass());
    fpm.add(llvm::createReassociatePass());
    fpm.add(llvm::createGVNPass());
    fpm.add(llvm::createCFGSimplificationPass());
    fpm.doInitialization();
    for (auto& f : module->getFunctionList()) {
        fpm.run(f);
    }

    auto tsm =
        llvm::orc::ThreadSafeModule(std::move(module), std::move(context));
    return Codegen2Module(std::move(tsm));
}

llvm::Function* Codegen2::start_function(const char* name,
                                         llvm::Type* ret_type) {
    auto fn_type = llvm::FunctionType::get(ret_type, false);
    auto fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage,
                                     name, *module);
    auto bb = llvm::BasicBlock::Create(*context, "entry", fn);
    builder->SetInsertPoint(bb);
    return fn;
}

llvm::Function* Codegen2::get_external_function(
    const char* name, llvm::Type* ret_type,
    std::vector<llvm::Type*> argument_types) {
    /* Try to get the function if it exists already. */
    /* WARNING: this will not make sure the types are matching! */
    llvm::Function* function = module->getFunction(name);
    if (function) return function;

    /* If it doesnt exist yet, create it. */
    auto malloc_type = llvm::FunctionType::get(ret_type, argument_types, false);
    return llvm::Function::Create(malloc_type, llvm::Function::ExternalLinkage,
                                  name, *module);
}

llvm::Function* Codegen2::get_crt_malloc() {
    return get_external_function("crt_malloc",
                                 llvm::PointerType::get(*context, 0),
                                 {llvm::Type::getInt64Ty(*context)});
}

llvm::Function* Codegen2::get_crt_string__concat() {
    llvm::Type* ptrt = CRT_LLVM_TYPE(crt_string, *context)->getPointerTo();
    return get_external_function("crt_string__concat", ptrt, {ptrt, ptrt});
}

/* ----------------- visitor functions -------------------*/
void Codegen2::visit_exprstmt(ExprStmt* exprstmt) {}

void Codegen2::visit_binary(Binary* binary) {
    auto op_token = binary->get_op().type;
    llvm::Value* lhs = do_visit(binary->get_lhs());
    llvm::Value* rhs = nullptr;

    auto lhs_btype = binary->get_lhs()->get_type()->get_base_type();
    auto rhs_btype = binary->get_rhs()->get_type()->get_base_type();

    /* For short circuiting, dont eval rhs now if we have bools */
    if (lhs_btype != types::BaseType::BOOL) {
        rhs = do_visit(binary->get_rhs());
    }

    switch (lhs_btype) {
        case types::BaseType::INT:
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
                    error("unsupported int binop found");
                    break;
            }
            break;
        case types::BaseType::FLOAT:
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
                    error("unsupported float binop found");
                    break;
            }
            break;
        case types::BaseType::STRING:
            result = builder->CreateCall(get_crt_string__concat(), {lhs, rhs});
            break;
        case types::BaseType::BOOL:
        case types::BaseType::FN:
            error("binary lhs type not implemented");
            break;
        default:
            error("Unexpected binary expr lhs type");
    }
}

void Codegen2::visit_number(Number* number) {
    llvm::Type* llvm_type = number->get_type()->get_llvm_rt_type(*context);
    switch (number->get_type()->get_base_type()) {
        case types::BaseType::INT:
            result = llvm::ConstantInt::get(llvm_type,
                                            atoi(number->get_value().start));
            break;
        case types::BaseType::FLOAT:
            result = llvm::ConstantFP::get(
                llvm_type, (double)atof(number->get_value().start));
            break;
        default:
            error("Invalid number base type encountered.");
    }
}

void Codegen2::visit_string(String* string) {
    /* Allocate the new crt_string object. */
    llvm::Function* fn_crt_alloc = get_crt_malloc();
    llvm::Value* crt_string_ptr = builder->CreateCall(
        fn_crt_alloc, mk_uint64(sizeof(crt_string)), "crt_malloc");

    /* Initialize the crt_string. */
    std::string std_string = std::string(string->get_value().start + 1,
                                         string->get_value().length - 2);
    auto string_ref = llvm::StringRef(std_string);
    auto const_str = llvm::ConstantDataArray::getString(*context, string_ref);
    llvm::Value* str_len = llvm::ConstantInt::get(
        llvm::Type::getInt64Ty(*context), std_string.size() + 1, false);
    llvm::Value* crt_string_data_ptr =
        builder->CreateCall(fn_crt_alloc, {str_len}, "crt_malloc");
    builder->CreateStore(const_str, crt_string_data_ptr);

    /* Set fields of crt string */
    auto* gep_len =
        builder->CreateGEP(CRT_LLVM_TYPE(crt_string, *context), crt_string_ptr,
                           {mk_uint32(0), mk_uint32(0)}, "crt_string_len");
    auto* gep_data =
        builder->CreateGEP(CRT_LLVM_TYPE(crt_string, *context), crt_string_ptr,
                           {mk_uint32(0), mk_uint32(1)}, "crt_string_data");
    builder->CreateStore(str_len, gep_len);
    builder->CreateStore(crt_string_data_ptr, gep_data);

    result = crt_string_ptr;
}
