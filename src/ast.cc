#include "carl/ast/ast.h"

#include <iostream>

namespace carl {

void Binary::accept(AstNodeVisitor* visitor) { visitor->visit_binary(this); }
void Unary::accept(AstNodeVisitor* visitor) { visitor->visit_unary(this); }
void Variable::accept(AstNodeVisitor* visitor) {
    visitor->visit_variable(this);
}
void Literal::accept(AstNodeVisitor* visitor) { visitor->visit_literal(this); }
void String::accept(AstNodeVisitor* visitor) { visitor->visit_string(this); }
void Number::accept(AstNodeVisitor* visitor) { visitor->visit_number(this); }

}  // namespace carl