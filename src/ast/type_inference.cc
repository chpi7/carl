#include "carl/ast/type_inference.h"

using namespace carl;

TypeInference::TypeInference() {
    env = std::make_unique<Environment<std::shared_ptr<types::Type>,
                                       std::shared_ptr<types::Type>>>(nullptr);
}

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
    if (error) {
        return TypeInferenceResult::make_error(error.value());
    }
    return TypeInferenceResult::make_result(nullptr);
}

void TypeInference::visit_type(Type* type) {}

void TypeInference::visit_formalparam(FormalParam* formalparam) {
    auto name = std::string(formalparam->get_name().start, formalparam->get_name().length);
    env->set_variable(name, formalparam->get_type());
}

void TypeInference::visit_fndecl(FnDecl* fndecl) {
    // register fn in current env
    auto fname = std::string(fndecl->get_name().start, fndecl->get_name().length);
    env->set_function(fname, fndecl->get_type());

    // then go into inside env for the function where the formals are known
    UseNewEnv _(env.get());

    for (auto& fp : fndecl->get_formals()) do_visit(fp);
    do_visit(fndecl->get_body());

    result = fndecl->get_type();
}

void TypeInference::visit_letdecl(LetDecl* letdecl) {
    auto initializer_type = do_visit(letdecl->get_initializer());
    env->set_variable(letdecl->get_name(), letdecl->get_type());
    if (initializer_type->get_base_type() == types::BaseType::UNKNOWN) {
        report_error("Could not get let initializer type");
        return;
    }
    if (initializer_type->get_base_type() == types::BaseType::VOID) {
        report_error("Invalid initializer type " + initializer_type->str());
        return;
    }
    letdecl->set_type(initializer_type);
    env->set_variable(letdecl->get_name(), letdecl->get_type());
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
    UseNewEnv _(env.get());
    for (auto& decl : block->get_declarations()) do_visit(decl);
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
                result = std::make_shared<types::Bool>();
            } else if (type_l->is_number() && type_r->is_number()) {
                // non equal number types
                if (type_r->can_cast_to(type_l.get())) {
                    result = std::make_shared<types::Bool>();
                } else if (type_l->can_cast_to(type_r.get())) {
                    result = std::make_shared<types::Bool>();
                } else {
                    report_error("Can not cast number types in logic binop.");
                    return;
                }
            } else {
                report_error("Types " + type_l->str() + " and " + type_r->str() + " are not compatible in logic binop.");
                return;
            }
            break;
        case TOKEN_DOT: {
            // function composition f(g(x)) = (f . g)(x) (like haskell)
            // so . does (b -> c) -> (a -> b) -> (a -> c)
            if (type_l->get_base_type() != types::BaseType::FN) {
                report_error("lhs of . needs to a function but it is " + type_l->str());
            }
            if (type_r->get_base_type() != types::BaseType::FN) {
                report_error("rhs of . needs to a function but it is " + type_r->str());
            }
            if (error) break; // dont cast if there is an error --> will segfault
            auto fntl = std::reinterpret_pointer_cast<types::Fn>(type_l);
            auto fntr = std::reinterpret_pointer_cast<types::Fn>(type_r);
            if (!fntl->can_apply_to(std::vector{fntr->get_ret()})) {
                report_error("Can not pass return type " +
                             fntr->get_ret()->str() + " into " + fntl->str());
            }
            result = std::make_shared<types::Fn>(fntr->get_parameters(), fntl->get_ret());
            break;
        }
        default:
            report_error("unsupported binop " + std::string(binary->get_op()) + " for type inference");
    }
    binary->set_type(result);
}

void TypeInference::visit_unary(Unary* unary) {
    result = do_visit(unary->get_operand());
    unary->set_type(result);
}

void TypeInference::visit_variable(Variable* variable) {
    std::string vname = variable->get_name();
    std::shared_ptr<types::Type> vtype;
    if (env->has_function(vname)) {
        vtype = env->get_function(vname);
    } else if (env->has_variable(vname)) {
        vtype = env->get_variable(vname);
    } else {
        report_error("Can not find variable or function with name " + vname);
        return;
    }
    variable->set_type(vtype);
    result = vtype;
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
    std::string callee_name = call->get_fname();
    std::shared_ptr<types::Type> callee_type;
    if (env->has_function(callee_name)) {
        callee_type = env->get_function(callee_name);
    } else if (env->has_variable(callee_name)) {
        callee_type = env->get_variable(callee_name);
    } else {
        report_error("Can not find callable with name " + callee_name);
        return;
    }

    if (callee_type->get_base_type() != types::BaseType::FN) {
        report_error(std::string(callee_name) + "is not of basetype fn.");
        return;
    }

    auto fn_type = std::reinterpret_pointer_cast<types::Fn>(callee_type);

    if (fn_type->get_parameters().size() != call->get_arguments().size()) {
        report_error("Argument number mismatch.");
    }

    std::vector<std::shared_ptr<types::Type>> arg_types;
    for (auto& arg : call->get_arguments()) {
        do_visit(arg);
        arg_types.push_back(arg->get_type());
    }

    if (!fn_type->can_apply_to(arg_types)) {
        report_error("Arguments are not of the expected type for " + fn_type->str());
    }

    call->set_type(fn_type->get_ret());
    result = call->get_type();
}
