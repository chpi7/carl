#include "carl/ast/ast.h"

namespace carl {

void Statement::accept(AstNodeVisitor* visitor) { visitor->visit_statement(this); }
void Block::accept(AstNodeVisitor* visitor) { visitor->visit_block(this); }
void Expression::accept(AstNodeVisitor* visitor) { visitor->visit_expression(this); }
void Type::accept(AstNodeVisitor* visitor) { visitor->visit_type(this); }
void FormalParam::accept(AstNodeVisitor* visitor) { visitor->visit_formalparam(this); }
void FnDecl::accept(AstNodeVisitor* visitor) { visitor->visit_fndecl(this); }
void LetDecl::accept(AstNodeVisitor* visitor) { visitor->visit_letdecl(this); }
void ExprStmt::accept(AstNodeVisitor* visitor) { visitor->visit_exprstmt(this); }
void ReturnStmt::accept(AstNodeVisitor* visitor) { visitor->visit_returnstmt(this); }
void WhileStmt::accept(AstNodeVisitor* visitor) { visitor->visit_whilestmt(this); }
void Assignment::accept(AstNodeVisitor* visitor) { visitor->visit_assignment(this); }
void Binary::accept(AstNodeVisitor* visitor) { visitor->visit_binary(this); }
void Unary::accept(AstNodeVisitor* visitor) { visitor->visit_unary(this); }
void Variable::accept(AstNodeVisitor* visitor) { visitor->visit_variable(this); }
void Literal::accept(AstNodeVisitor* visitor) { visitor->visit_literal(this); }
void String::accept(AstNodeVisitor* visitor) { visitor->visit_string(this); }
void Number::accept(AstNodeVisitor* visitor) { visitor->visit_number(this); }
void Call::accept(AstNodeVisitor* visitor) { visitor->visit_call(this); }
void PartialApp::accept(AstNodeVisitor* visitor) { visitor->visit_partialapp(this); }

AstNodeType Statement::get_node_type() { return AstNodeType::Statement; }
AstNodeType Block::get_node_type() { return AstNodeType::Block; }
AstNodeType Expression::get_node_type() { return AstNodeType::Expression; }
AstNodeType Type::get_node_type() { return AstNodeType::Type; }
AstNodeType FormalParam::get_node_type() { return AstNodeType::FormalParam; }
AstNodeType FnDecl::get_node_type() { return AstNodeType::FnDecl; }
AstNodeType LetDecl::get_node_type() { return AstNodeType::LetDecl; }
AstNodeType ExprStmt::get_node_type() { return AstNodeType::ExprStmt; }
AstNodeType ReturnStmt::get_node_type() { return AstNodeType::ReturnStmt; }
AstNodeType WhileStmt::get_node_type() { return AstNodeType::WhileStmt; }
AstNodeType Assignment::get_node_type() { return AstNodeType::Assignment; }
AstNodeType Binary::get_node_type() { return AstNodeType::Binary; }
AstNodeType Unary::get_node_type() { return AstNodeType::Unary; }
AstNodeType Variable::get_node_type() { return AstNodeType::Variable; }
AstNodeType Literal::get_node_type() { return AstNodeType::Literal; }
AstNodeType String::get_node_type() { return AstNodeType::String; }
AstNodeType Number::get_node_type() { return AstNodeType::Number; }
AstNodeType Call::get_node_type() { return AstNodeType::Call; }
AstNodeType PartialApp::get_node_type() { return AstNodeType::PartialApp; }

} // namespace carl
