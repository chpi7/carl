#pragma once

#include <memory>
#include <vector>

#include "carl/ast/ast.h"
#include "carl/name_environment.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"

namespace carl {

/* Wrapper for code generator result. */
class Codegen2Module {
   private:
    llvm::orc::ThreadSafeModule module;

   public:
    Codegen2Module(llvm::orc::ThreadSafeModule&& tsm)
        : module(std::move(tsm)){};

    llvm::orc::ThreadSafeModule&& take_llvm_module() {
        return std::move(module);
    }
};

class Value {
    public:
    bool is_alloca = false;
    bool is_global = false;
    private:
    llvm::Value* value = nullptr;

   public:
    Value(llvm::AllocaInst* local) { 
        is_alloca = true;
        value = local;
    }
    llvm::AllocaInst* as_alloca() const {
        return static_cast<llvm::AllocaInst*>(value);
    }
    llvm::Value* get_value() const {
        return value;
    }
    llvm::Type* get_type() const {
        if (is_alloca) return static_cast<llvm::AllocaInst*>(value)->getAllocatedType();
        // TODO: check if this is the correct type to cast to!
        else if (is_global) return static_cast<llvm::GlobalVariable*>(value)->getValueType();
        return nullptr;
    }
};

class Codegen2 : public AstNodeVisitor {
   private:
    llvm::Value* result;
    bool has_error = false;
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;

    Environment<Value> named_values;

   public:
    Codegen2();
    void init(std::string module_name);
    Codegen2Module generate(std::vector<std::shared_ptr<AstNode>> declarations);

   private:
    void error(const char* error) {
        result = nullptr;
        has_error = true;
        fprintf(stderr, "Codegen Error: %s\n", error);
    }
    llvm::Value* do_visit(std::shared_ptr<AstNode> node) {
        node->accept(this);
        return result;
    }
    llvm::Value* mk_uint64(uint64_t i) {
        return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), i,
                                      false);
    }
    llvm::Value* mk_uint32(int i) {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), i,
                                      false);
    }
    llvm::AllocaInst* create_alloca(std::string name, llvm::Type* type) {
        llvm::BasicBlock* bb = builder->GetInsertBlock();
        llvm::IRBuilder<> tmp_builder(bb, bb->begin());
        return tmp_builder.CreateAlloca(type, nullptr, name);
    }
    llvm::Function* get_external_function(
        const char* name, llvm::Type* ret_type,
        std::vector<llvm::Type*> argument_types);
    llvm::Function* get_crt_malloc();
    llvm::Function* get_crt_string__concat();
    llvm::Function* start_function(const char* name, llvm::Type* ret_type);

    /* --------------- visitor methods -------------- */
    void visit_exprstmt(ExprStmt* exprstmt);
    void visit_binary(Binary* binary);
    void visit_number(Number* number);
    void visit_string(String* number);
    void visit_letdecl(LetDecl* letdecl);
    void visit_variable(Variable* variable);
    void visit_returnstmt(ReturnStmt* returnstmt);
    void visit_fndecl(FnDecl* fndecl);
    void visit_block(Block* block);
    void visit_call(Call* call);
};

}  // namespace carl