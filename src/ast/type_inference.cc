#include "carl/ast/type_inference.h"

using namespace carl;

TypeInferenceResult TypeInference::run(
    std::vector<std::shared_ptr<AstNode>> decls) {
    clear_error();
    for (auto& decl : decls) {
        auto result = run(decl);
        if (!result) return result;
    }
    return TypeInferenceResult::make_result(nullptr);
}

TypeInferenceResult TypeInference::run(std::shared_ptr<AstNode> decl) {
    do_visit(decl);
    return TypeInferenceResult::make_result(nullptr);
}

void TypeInference::visit_type(Type* type) {}

void TypeInference::visit_formalparam(FormalParam* formalparam) {
    // n.a. (happens during parsing)
}

void TypeInference::visit_fndecl(FnDecl* fndecl) {
    // n.a. (happens during parsing)
    result = fndecl->get_type();
}

void TypeInference::visit_letdecl(LetDecl* letdecl) {
    auto initializer_type = do_visit(letdecl->get_initializer());
    if (initializer_type->get_base_type() == types::BaseType::UNKNOWN) {
        report_error("Could not get let initializer type");
        return;
    }
    letdecl->set_type(initializer_type);
}

void TypeInference::visit_exprstmt(ExprStmt* exprstmt) {
    do_visit(exprstmt->get_expr());
}

void TypeInference::visit_returnstmt(ReturnStmt* returnstmt) {
    do_visit(returnstmt->get_expr());
}

void TypeInference::visit_whilestmt(WhileStmt* whilestmt) {
    auto cond_t = do_visit(whilestmt->get_condition());
    if (cond_t->get_base_type() != types::BaseType::BOOL) {
        report_error("while condition type should be bool but is " +
                     cond_t->str());
    }
    do_visit(whilestmt->get_body());
}

void TypeInference::visit_block(Block* block) {
    for (auto& decl : block->get_declarations()) {
        do_visit(decl);
    }
}

void TypeInference::visit_assignment(Assignment* assignment) {
    auto target_type = do_visit(assignment->get_target());
    auto val_type = do_visit(assignment->get_expr());
    if (!target_type->can_assign(val_type.get())) {
        report_error("Can not assign " + val_type->str() + " to " + target_type->str());
    }
    assignment->set_type(target_type); // "assignees" can never change their type.
    result = target_type;
}

void TypeInference::visit_binary(Binary* binary) {
    auto op_token = binary->get_op().type;
    auto type_l = do_visit(binary->get_lhs());
    auto type_r = do_visit(binary->get_rhs());

    switch (op_token) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_STAR:
        case TOKEN_SLASH:
            // arithmetic op
            if (!type_l->is_number()) {
                report_error(type_l->str() + " should be a number.");
                return;
            }
            if (!type_r->is_number()) {
                report_error(type_r->str() + " should be a number.");
                return;
            }

            if (type_l->equals(type_r.get())) {
                result = type_l;
            } else if (type_r->can_cast_to(type_l.get())) {
                result = type_l;
            } else if (type_l->can_cast_to(type_r.get())) {
                result = type_r;
            } else {
                report_error("Types " + type_l->str() + " and " + type_r->str() + " are not compatible in arith binop.");
                return;
            }
            break;
        case TOKEN_EQUAL_EQUAL:
        case TOKEN_BANG_EQUAL:
        case TOKEN_LESS:
        case TOKEN_LESS_EQUAL:
        case TOKEN_GREATER:
        case TOKEN_GREATER_EQUAL:
            // logic op --> only allow comparison of equal types or numbers
            if (type_l->equals(type_r.get())) {
                result = type_l;
            } else if (type_l->is_number() && type_r->is_number()) {
                // non equal number types
                if (type_r->can_cast_to(type_l.get())) {
                    result = type_l;
                } else if (type_l->can_cast_to(type_r.get())) {
                    result = type_r;
                } else {
                    report_error("Can not cast number types in logic binop.");
                    return;
                }
            } else {
                report_error("Types " + type_l->str() + " and " + type_r->str() + " are not compatible in logic binop.");
                return;
            }
            break;
        default:
            break;
    }
    binary->set_type(result);
}

void TypeInference::visit_unary(Unary* unary) {
    result = do_visit(unary->get_operand());
    unary->set_type(result);
}

void TypeInference::visit_variable(Variable* variable) {
    result = std::make_shared<types::Unknown>();
}

void TypeInference::visit_literal(Literal* literal) {
    result = literal->get_type();
}

void TypeInference::visit_string(String* string) {
    result = string->get_type();
}

void TypeInference::visit_number(Number* number) {
    result = number->get_type();
}

void TypeInference::visit_call(Call* call) {
    result = std::make_shared<types::Unknown>();
    // auto call_type = std::reinterpret_pointer_cast<types::Fn>(call->get_type());
    // if (call_type->get_ret().has_value()) {
    //     result = call_type->get_ret().value();
    // } else {
    // }
}
