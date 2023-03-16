#include "carl/ast/type_inference.h"

using namespace carl;

static std::vector<std::shared_ptr<ReturnStmt>> find_returns(std::shared_ptr<Block> block) {
    std::vector<std::shared_ptr<ReturnStmt>> result;
    for (auto& decl : block->get_declarations()) {
        if (decl->get_node_type() == AstNodeType::ReturnStmt) {
            result.push_back(std::reinterpret_pointer_cast<ReturnStmt>(decl));
        }
    }
    return result;
}

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

    result = fndecl->get_type();

    // then go into inside env for the function where the formals are known
    UseNewEnv _(env.get());

    for (auto& fp : fndecl->get_formals()) do_visit(fp);
    do_visit(fndecl->get_body());

    auto fntype = std::reinterpret_pointer_cast<types::Fn>(fndecl->get_type());
    auto returns = find_returns(fndecl->get_body());
    if (fntype->get_ret()->get_base_type() != types::BaseType::VOID) {
        auto expected_ret_type = fntype->get_ret();
        for (auto& ret : returns) {
            auto returned_type = ret->get_expr()->get_type();
            if (!returned_type->can_cast_to(expected_ret_type.get())) {
                report_error("Return type of " + fname + " is " + expected_ret_type->str() + " but return gives " + returned_type->str());
            }
        }
    } else if (!returns.size() == 0) {
        report_error("Return statement found in function with return type void.");
    } else {
        // void with no returns --> okay
    }
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
            // for + we accept string as well
            if (type_l->get_base_type() == types::BaseType::STRING &&
                type_r->get_base_type() == types::BaseType::STRING) {
                result = type_l;
                break;
            }
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
                report_error("Can not compose " + fntl->str() + " . " + fntr->str(), binary->get_op());
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

void TypeInference::visit_partialapp(PartialApp* partialapp) {
    std::string target_name = partialapp->get_fname();
    std::shared_ptr<types::Fn> orig_fn_type;
    if (env->has_variable(target_name)) {
        auto x = env->get_variable(target_name);
        if (x->get_base_type() != types::BaseType::FN) {
            report_error("partial app target name is not a function, it is " + x->str());
        } else {
            orig_fn_type = std::reinterpret_pointer_cast<types::Fn>(x);
        }
    } else if (env->has_function(target_name)) {
        orig_fn_type = std::reinterpret_pointer_cast<types::Fn>(
            env->get_function(target_name));
    } else {
        report_error("Name " + target_name + " not found in env for partialapp.");
    }

    std::vector<std::shared_ptr<types::Type>> new_params;
    std::vector<int> defined_param_indices;
    int current_idx = 0;
    for (auto idx : partialapp->get_placeholder_positions()) {
        while (current_idx < idx) defined_param_indices.push_back(current_idx++);
        new_params.push_back(orig_fn_type->get_parameters().at(idx));
        current_idx = idx + 1;
    }
    while (current_idx < orig_fn_type->get_parameters().size())
        defined_param_indices.push_back(current_idx++);

    current_idx = 0;
    for (auto& given_arg: partialapp->get_arguments()) {
        auto orig_param_idx = defined_param_indices.at(0);
        auto& required_type = orig_fn_type->get_parameters().at(orig_param_idx);
        if (!given_arg->get_type()->can_cast_to(required_type.get())) {
            report_error("Can not cast " + given_arg->get_type()->str() +
                         " to " + required_type->str() + " at arg idx " +
                         std::to_string(orig_param_idx) + " of " +
                         orig_fn_type->str());
        }
        current_idx++;
    }
    if (!error) {
        result = std::make_shared<types::Fn>(new_params, orig_fn_type->get_ret());
    }
}
