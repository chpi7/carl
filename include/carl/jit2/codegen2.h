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
        return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), i, false);
    }
    llvm::Value* mk_uint32(int i) {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), i, false);
    }
    llvm::Function* get_external_function(const char* name, llvm::Type* ret_type, std::vector<llvm::Type*> argument_types);
    llvm::Function* get_crt_malloc();
    llvm::Function* get_crt_string__concat();
    llvm::Function* start_function(const char* name, llvm::Type* ret_type);

    /* --------------- visitor methods -------------- */
    void visit_exprstmt(ExprStmt* exprstmt);
    void visit_binary(Binary* binary);
    void visit_number(Number* number);
    void visit_string(String* number);
};

}  // namespace carl