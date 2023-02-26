#include "carl/ast/ast.h"

namespace carl {

void Invalid::accept(AstNodeVisitor* visitor) { visitor->visit_invalid(this); }
void ExprStmt::accept(AstNodeVisitor* visitor) { visitor->visit_exprstmt(this); }
void WhileStmt::accept(AstNodeVisitor* visitor) { visitor->visit_whilestmt(this); }
void Block::accept(AstNodeVisitor* visitor) { visitor->visit_block(this); }
void LetStmt::accept(AstNodeVisitor* visitor) { visitor->visit_letstmt(this); }
void Assignment::accept(AstNodeVisitor* visitor) { visitor->visit_assignment(this); }
void Binary::accept(AstNodeVisitor* visitor) { visitor->visit_binary(this); }
void Unary::accept(AstNodeVisitor* visitor) { visitor->visit_unary(this); }
void Variable::accept(AstNodeVisitor* visitor) { visitor->visit_variable(this); }
void Literal::accept(AstNodeVisitor* visitor) { visitor->visit_literal(this); }
void String::accept(AstNodeVisitor* visitor) { visitor->visit_string(this); }
void Number::accept(AstNodeVisitor* visitor) { visitor->visit_number(this); }

} // namespace carl
