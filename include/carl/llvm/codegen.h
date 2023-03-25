#ifndef carl_llvm_codegen_h
#define carl_llvm_codegen_h

#include <map>
#include <memory>

#include "carl/ast/ast.h"
#include "carl/name_environment.h"

#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"

namespace carl {

struct TypedValue {
    // the typed value
    llvm::Value* value;
    // the type of the allocated (contained) object, not the object itself! For
    // locals it is the alloca->getalloctedtype and for globals the other one.
    llvm::Type* type;
    // is it an alloca or a global?
    bool local;

    // operator llvm::Value*() const noexcept { return value; }

    TypedValue() {
        value = nullptr;
        type = nullptr;
        local = false;
    }

    TypedValue(const TypedValue& other) {
        this->value = other.value;
        this->type = other.type;
        this->local = other.local;
    }

    TypedValue(TypedValue&& other) {
        this->value = other.value;
        this->type = other.type;
        this->local = other.local;
    }

    TypedValue(llvm::AllocaInst* v) {
        this->value = v;
        this->type = v->getAllocatedType();
        this->local = true;
    }

    TypedValue(llvm::GlobalVariable* v) {
        this->value = v;
        this->type = v->getValueType();
        this->local = false;
    }

    TypedValue& operator=(const TypedValue& other) {
        if (this == &other) {
            return *this;
        }
        this->value = other.value;
        this->type = other.type;
        this->local = other.local;

        return *this;
    }
};

class LLVMCodeGenerator : public AstNodeVisitor {
   private:
    std::unique_ptr<Environment<TypedValue>> names;
    bool has_error;
    llvm::Value* result;
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;

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