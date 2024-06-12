#include <gtest/gtest.h>

#include "carl/experimental/type.h"

using namespace carl::polymorphic;

namespace {
TEST(PolymorphicTypes, playground) {
    // ident x -> x
    std::string fname = "ident";
    ref<ast::Expr> ident = ast::Expr::make<ast::Fn>(fname, "x", ast::Expr::make<ast::Var>("x"));
    // ident ident 2
    ref<ast::Expr> ident_to_int_expr = ast::Expr::make<ast::Application>(
        ast::Expr::make<ast::Application>(ast::Expr::make<ast::Var>(fname), ast::Expr::make<ast::Var>(fname)),
        ast::Expr::make<ast::IntegerConst>(2));
    // ident ident ident
    ref<ast::Expr> ident_to_fn_expr = ast::Expr::make<ast::Application>(
        ast::Expr::make<ast::Application>(ast::Expr::make<ast::Var>(fname), ast::Expr::make<ast::Var>(fname)),
        ast::Expr::make<ast::Var>(fname));

    TypeChecker checker{};
    ref<Type> ident_type = checker.check(ident, checker.globals, /*store_type=*/true);
    ref<Type> int_expr_type = checker.check(ident_to_int_expr, checker.globals);
    ref<Type> fn_expr_type = checker.check(ident_to_fn_expr, checker.globals);

    // type of (ident ident 2) == Integer
    ASSERT_EQ(int_expr_type->get_basic_type(), Type::BasicType::Integer);

    // type of (ident ident ident) == 'A -> 'A
    ASSERT_TRUE(fn_expr_type->is_function());
    ref<Function> f = std::dynamic_pointer_cast<Function>(fn_expr_type);
    ASSERT_TRUE(f->domain->is_var());
    ASSERT_TRUE(f->image->is_var());
    ASSERT_EQ(f->image->eq_class, f->domain->eq_class);
}
}  // namespace
