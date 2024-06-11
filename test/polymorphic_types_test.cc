#include <gtest/gtest.h>

#include "carl/experimental/type.h"

using namespace carl::polymorphic;

namespace {
TEST(PolymorphicTypes, playground) {
    // ident x -> x
    std::string fname = "ident";
    ref<Expr> ident = get_Fn(fname, "x", get_Var("x"));
    // ident ident 2
    ref<Expr> expr = get_Application(
        get_Application(get_Var(fname), get_Var(fname)), get_IntegerConst(2));

    ident->println(std::cout);
    expr->println(std::cout);

    Env env;
    ref<Type> ident_type = check(ident, env);
    printf("--- 1\n");
    // store type of this declaration in top level env for future use.
    env.update("ident", ident_type);
    printf("--- 2\n");
    ref<Type> expr_type = check(expr, env);
    /*
    ident_type->println(std::cout);
    expr_type->println(std::cout);

    std::cout << "Environment mappings:\n";
    for (const auto &kvp : env.mappings) {
        std::cout << kvp.first << ": ";
        kvp.second->println(std::cout);
    }
    */
}
}  // namespace
