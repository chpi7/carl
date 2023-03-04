#include "carl/ast/ast.h"

namespace carl {

void Invalid::accept(AstNodeVisitor* visitor) { visitor->visit_invalid(this); }
void Statement::accept(AstNodeVisitor* visitor) { visitor->visit_statement(this); }
void Expression::accept(AstNodeVisitor* visitor) { visitor->visit_expression(this); }
void Type::accept(AstNodeVisitor* visitor) { visitor->visit_type(this); }
void FormalParam::accept(AstNodeVisitor* visitor) { visitor->visit_formalparam(this); }
void FnDecl::accept(AstNodeVisitor* visitor) { visitor->visit_fndecl(this); }
void LetStmt::accept(AstNodeVisitor* visitor) { visitor->visit_letstmt(this); }
void ExprStmt::accept(AstNodeVisitor* visitor) { visitor->visit_exprstmt(this); }
void ReturnStmt::accept(AstNodeVisitor* visitor) { visitor->visit_returnstmt(this); }
void WhileStmt::accept(AstNodeVisitor* visitor) { visitor->visit_whilestmt(this); }
void Block::accept(AstNodeVisitor* visitor) { visitor->visit_block(this); }
void Assignment::accept(AstNodeVisitor* visitor) { visitor->visit_assignment(this); }
void Binary::accept(AstNodeVisitor* visitor) { visitor->visit_binary(this); }
void Unary::accept(AstNodeVisitor* visitor) { visitor->visit_unary(this); }
void Variable::accept(AstNodeVisitor* visitor) { visitor->visit_variable(this); }
void Literal::accept(AstNodeVisitor* visitor) { visitor->visit_literal(this); }
void String::accept(AstNodeVisitor* visitor) { visitor->visit_string(this); }
void Number::accept(AstNodeVisitor* visitor) { visitor->visit_number(this); }
void Call::accept(AstNodeVisitor* visitor) { visitor->visit_call(this); }

} // namespace carl
