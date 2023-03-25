#ifndef carl_type_inference_h 
#define carl_type_inference_h

#include <vector>
#include <memory>
#include <string>
#include <optional>

#include "carl/ast/ast.h"
#include "carl/name_environment.h"

namespace carl {

struct TypeInferenceError {
    std::string message;
};

using TypeInferenceResult = Result<nullptr_t, TypeInferenceError>;

class TypeInference : public AstNodeVisitor {
   private:
    std::unique_ptr<Environment<std::shared_ptr<types::Type>>> env;
    std::unique_ptr<Environment<std::shared_ptr<types::Type>>> fn_env;
    std::shared_ptr<types::Type> result;
    std::optional<TypeInferenceError> error;

   private:
    std::shared_ptr<types::Type> do_visit(std::shared_ptr<AstNode> node) {
        node->accept(this);
        return result;
    }

    void report_error(const char* msg) {
        report_error(std::string(msg));
    }

    void report_error(std::string msg, Token token = {}) {
        if (token.length > 0) {
            std::cerr << "[line " << token.line << "] Error at '" << std::string(token) << "': ";
        }
        std::cerr << msg << std::endl;
        result = std::make_shared<types::Unknown>();
        if (error) return;  // keep initial error.
        error = TypeInferenceError{.message = msg};
    }

    void clear_error() { error = std::nullopt; }

   public:
    TypeInference();
    TypeInferenceResult run(std::shared_ptr<AstNode> decl);
    TypeInferenceResult run(std::vector<std::shared_ptr<AstNode>> decls);
    void visit_type(Type* type);
    void visit_formalparam(FormalParam* formalparam);
    void visit_fndecl(FnDecl* fndecl);
    void visit_letdecl(LetDecl* letdecl);
    void visit_exprstmt(ExprStmt* exprstmt);
    void visit_returnstmt(ReturnStmt* returnstmt);
    void visit_whilestmt(WhileStmt* whilestmt);
    void visit_block(Block* block);
    void visit_assignment(Assignment* assignment);
    void visit_binary(Binary* binary);
    void visit_unary(Unary* unary);
    void visit_variable(Variable* variable);
    void visit_literal(Literal* literal);
    void visit_string(String* string);
    void visit_number(Number* number);
    void visit_call(Call* call);
    void visit_partialapp(PartialApp* partialapp);
};
}  // namespace carl
#endif