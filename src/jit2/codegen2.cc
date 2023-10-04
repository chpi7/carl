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
    llvm::Type* ret_type = llvm::Type::getVoidTy(*context);
    llvm::Function* main = start_function("__carl_main", ret_type);

    /* Generate code for everything */
    for (const auto& d : decls) {
        do_visit(d);
    }
    /* In case there is no return in the code, at one. */
    builder->CreateRetVoid();

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

void Codegen2::visit_letdecl(LetDecl* letdecl) {
    std::string name = letdecl->get_name();
    llvm::Value* initializer = do_visit(letdecl->get_initializer());
    llvm::AllocaInst* local = create_alloca(name, initializer->getType());
    builder->CreateStore(initializer, local);
    named_values.set_variable(name, local);

    result = nullptr;
}

void Codegen2::visit_variable(Variable* variable) {
    Value& v = named_values.get_variable(variable->get_name());
    result = builder->CreateLoad(v.get_type(), v.get_value());
}

void Codegen2::visit_returnstmt(ReturnStmt* returnstmt) {
    builder->CreateRet(do_visit(returnstmt->get_expr()));
    result = nullptr;
}

void Codegen2::visit_fndecl(FnDecl* fndecl) {
    /*
    For this code:
    let a = 1;
    fn foo(b) {
        return a + b;
    }

    Do:
    1) Generate function code
    function foo_impl (b : int, a: int) : int {
        return a + b;
    }
    2) Create the crt_fn object
    crt_fn {
        impl_ptr = foo;
        captures = [a = 1]; <<--- vector / array of 64 bit values
    }
    3) Store crt_fn object in alloca with correct name
    */

    /* 1) */
    /* Generate function if it does not exist yet */
    std::string fname = fndecl->get_sname();
    llvm::Function* llvm_fn = module->getFunction(fname);
    if (llvm_fn == nullptr) {
        assert(fndecl->get_type()->get_base_type() == types::BaseType::FN);

        auto carl_fn_type =
            std::static_pointer_cast<types::Fn>(fndecl->get_type());

        auto llvm_ret_type = runtime_type_llvm_get__from_BaseType(
            carl_fn_type->get_ret()->get_base_type(), *context);

        std::vector<llvm::Type*> llvm_param_types;
        for (auto carl_param_type : carl_fn_type->get_parameters()) {
            llvm_param_types.push_back(runtime_type_llvm_get__from_BaseType(
                carl_param_type->get_base_type(), *context));
        }
        for (auto capture : fndecl->get_captures()) {
            llvm_param_types.push_back(runtime_type_llvm_get__from_BaseType(
                capture->get_type()->get_base_type(), *context));
        }

        auto llvm_fn_type =
            llvm::FunctionType::get(llvm_ret_type, llvm_param_types, false);

        std::string fn_name = fname + "_impl";
        llvm_fn = llvm::Function::Create(
            llvm_fn_type, llvm::Function::ExternalLinkage, fn_name, *module);

        // set argument names
        int arg_idx = 0;
        for (auto& arg : fndecl->get_formals()) {
            llvm_fn->getArg(arg_idx)->setName(std::string(arg->get_name()));
            arg_idx++;
        }
        for (auto capture : fndecl->get_captures()) {
            llvm_fn->getArg(arg_idx++)->setName(
                std::string(capture->get_name()));
        }

        // Generate function implementation
        auto* old_insert_block = builder->GetInsertBlock();
        auto* body = llvm::BasicBlock::Create(*context, "entry", llvm_fn);
        builder->SetInsertPoint(body);
        named_values.push();

        size_t num_args =
            fndecl->get_formals().size() + fndecl->get_captures().size();
        for (size_t arg_idx = 0; arg_idx < num_args; ++arg_idx) {
            llvm::Argument* v = llvm_fn->getArg(arg_idx);
            std::string name = v->getName().str();
            llvm::AllocaInst* alloca = create_alloca(name, v->getType());
            builder->CreateStore(v, alloca);
            named_values.set_variable(name, Value(alloca));
        }
        do_visit(fndecl->get_body());
        builder->CreateRetVoid();

        named_values.pop();
        builder->SetInsertPoint(old_insert_block);
    }

    /* 2) */
    llvm::Function* fn_crt_alloc = get_crt_malloc();
    auto* fn_llvm_type = CRT_LLVM_TYPE(crt_fn, *context);
    llvm::Value* crt_fn_ptr = builder->CreateCall(
        fn_crt_alloc, llvm::ConstantExpr::getSizeOf(fn_llvm_type),
        "crt_fn_ptr");
    auto* fn_ptr =
        builder->CreateGEP(fn_llvm_type, crt_fn_ptr,
                           {mk_uint32(0), mk_uint32(0)}, "crt_fn_fn_ptr");
    builder->CreateStore(llvm_fn, fn_ptr);

    // allocate captures vector
    auto* capture_ptr = builder->CreateCall(
        fn_crt_alloc,
        mk_uint64(sizeof(uint64_t) * fndecl->get_captures().size()),
        "capture_ptr");
    // capture values into the vector
    size_t capture_idx = 0;
    for (const auto& capture : fndecl->get_captures()) {
        auto& value = named_values.get_variable(capture->get_name());
        auto* loaded = builder->CreateLoad(value.get_type(), value.get_value());
        auto* capture_elem_ptr = builder->CreateGEP(
            llvm::Type::getInt64PtrTy(*context), capture_ptr,
            {mk_uint32(capture_idx)},
            std::string("capture_") + std::to_string(capture_idx));
        builder->CreateStore(loaded, capture_elem_ptr);

        capture_idx++;
    }
    // store capture ptr into the fn wrapper struct
    auto* fn_crt_capture_ptr =
        builder->CreateGEP(fn_llvm_type, crt_fn_ptr,
                           {mk_uint32(0), mk_uint32(1)}, "crt_fn_capture_ptr");
    builder->CreateStore(capture_ptr, fn_crt_capture_ptr);

    /* 3) */
    llvm::AllocaInst* fn_alloca =
        create_alloca(fndecl->get_sname(), crt_fn_ptr->getType());
    builder->CreateStore(crt_fn_ptr, fn_alloca);
    Value value(fn_alloca);
    value.fn_capture_amount = fndecl->get_captures().size();
    named_values.set_variable(fndecl->get_sname(), value);

    module->print(llvm::outs(), nullptr);
}

void Codegen2::visit_block(Block* block) {
    named_values.push();
    for (const auto& declaration : block->get_declarations()) {
        do_visit(declaration);
    }
    named_values.pop();
    result = nullptr;
}

void Codegen2::visit_call(Call* call) {
    Value& v = named_values.get_variable(call->get_fname());
    if (v.fn_capture_amount < 0) {
        error("invalid capture amount in value");
        return;
    }
    auto* fn_wrapper =
        builder->CreateLoad(v.get_type(), v.get_value(),
                            std::string(call->get_fname()) + "_wrapper");
    auto* fn_wrapper_fn_ptr_gep =
        builder->CreateGEP(CRT_LLVM_TYPE(crt_fn, *context), fn_wrapper,
                           {mk_uint32(0), mk_uint32(0)}, "fn_ptr_gep");
    auto* fn_impl_ptr = builder->CreateLoad(llvm::PointerType::get(*context, 0),
                                            fn_wrapper_fn_ptr_gep, "impl_ptr");
    std::vector<llvm::Value*> arguments;
    std::vector<llvm::Type*> arg_types;
    for (const auto& arg : call->get_arguments()) {
        auto* v = do_visit(arg);
        arguments.push_back(v);
        arg_types.push_back(v->getType());
    }

    auto* fn_wrapper_cap_gep =
        builder->CreateGEP(CRT_LLVM_TYPE(crt_fn, *context), fn_wrapper,
                           {mk_uint32(0), mk_uint32(1)}, "capture_gep");
    auto* fn_wrapper_cap_ptr = builder->CreateLoad(llvm::Type::getInt64PtrTy(*context), fn_wrapper_cap_gep, "capture_ptr");
    for (size_t cap_idx = 0; cap_idx < v.fn_capture_amount; ++cap_idx) {
        auto* capture_gep = builder->CreateGEP(
            llvm::Type::getInt64Ty(*context), fn_wrapper_cap_ptr,
            {mk_uint32(cap_idx)},
            std::string("capture_gep_") + std::to_string(cap_idx));
        auto* capture = builder->CreateLoad(llvm::Type::getInt64Ty(*context), capture_gep);
        arguments.push_back(capture);
        arg_types.push_back(capture->getType());
    }

    auto* ret_type = runtime_type_llvm_get__from_BaseType(
        call->get_type()->get_base_type(), *context);
    llvm::FunctionType* fn_type =
        llvm::FunctionType::get(ret_type, arg_types, false);
    result = builder->CreateCall(fn_type, fn_impl_ptr, arguments,
                                 std::string(call->get_fname()));
}
