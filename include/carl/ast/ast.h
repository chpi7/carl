#ifndef carl_ast_h
#define carl_ast_h

#include <sstream>
#include <fstream>
#include <memory>
#include <list>
#include "carl/scanner.h"
#include "carl/common.h"
#include "carl/ast/types.h"

namespace carl {

class AstNodeVisitor;

class AstNode {
   public:
    virtual ~AstNode() = default;
    virtual void accept(AstNodeVisitor* visitor) = 0;
};

class Statement : public AstNode {
   public:
    Statement() {}
    void accept(AstNodeVisitor* visitor);
};

class Expression : public AstNode {
   private:
    std::shared_ptr<types::Type> type;
   public:
    Expression() {
        this->type = std::make_shared<types::Unknown>();
    }
    std::shared_ptr<types::Type> get_type() { return this->type; }
    void set_type(std::shared_ptr<types::Type> type) { this->type = type;}
    void accept(AstNodeVisitor* visitor);
};

class Type : public AstNode {
   private:
    Token name;
   public:
    Type(Token name) : name(name) {}
    Token get_name() { return this->name; }
    void accept(AstNodeVisitor* visitor);
};

class FormalParam : public AstNode {
   private:
    Token name;
    std::shared_ptr<types::Type> type;
   public:
    FormalParam(Token name) : name(name) {
        this->type = std::make_shared<types::Unknown>();
    }
    Token get_name() { return this->name; }
    std::shared_ptr<types::Type> get_type() { return this->type; }
    void set_type(std::shared_ptr<types::Type> type) { this->type = type;}
    void accept(AstNodeVisitor* visitor);
};

class FnDecl : public AstNode {
   private:
    Token name;
    std::list<std::shared_ptr<FormalParam>> formals;
    std::shared_ptr<Statement> body;
    std::shared_ptr<types::Type> type;
   public:
    FnDecl(Token name, std::list<std::shared_ptr<FormalParam>> formals, std::shared_ptr<Statement> body) : name(name), formals(formals), body(body) {
        this->type = std::make_shared<types::Unknown>();
    }
    Token get_name() { return this->name; }
    std::list<std::shared_ptr<FormalParam>> get_formals() { return this->formals; }
    std::shared_ptr<Statement> get_body() { return this->body; }
    std::shared_ptr<types::Type> get_type() { return this->type; }
    void set_type(std::shared_ptr<types::Type> type) { this->type = type;}
    void accept(AstNodeVisitor* visitor);
};

class LetDecl : public AstNode {
   private:
    Token name;
    std::shared_ptr<Expression> initializer;
    std::shared_ptr<types::Type> type;
   public:
    LetDecl(Token name, std::shared_ptr<Expression> initializer) : name(name), initializer(initializer) {
        this->type = std::make_shared<types::Unknown>();
    }
    Token get_name() { return this->name; }
    std::shared_ptr<Expression> get_initializer() { return this->initializer; }
    std::shared_ptr<types::Type> get_type() { return this->type; }
    void set_type(std::shared_ptr<types::Type> type) { this->type = type;}
    void accept(AstNodeVisitor* visitor);
};

class ExprStmt : public Statement {
   private:
    std::shared_ptr<Expression> expr;
   public:
    ExprStmt(std::shared_ptr<Expression> expr) : expr(expr) {}
    std::shared_ptr<Expression> get_expr() { return this->expr; }
    void accept(AstNodeVisitor* visitor);
};

class ReturnStmt : public Statement {
   private:
    std::shared_ptr<Expression> expr;
   public:
    ReturnStmt(std::shared_ptr<Expression> expr) : expr(expr) {}
    std::shared_ptr<Expression> get_expr() { return this->expr; }
    void accept(AstNodeVisitor* visitor);
};

class WhileStmt : public Statement {
   private:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> body;
   public:
    WhileStmt(std::shared_ptr<Expression> condition, std::shared_ptr<Statement> body) : condition(condition), body(body) {}
    std::shared_ptr<Expression> get_condition() { return this->condition; }
    std::shared_ptr<Statement> get_body() { return this->body; }
    void accept(AstNodeVisitor* visitor);
};

class Block : public Statement {
   private:
    std::list<std::shared_ptr<AstNode>> declarations;
   public:
    Block(std::list<std::shared_ptr<AstNode>> declarations) : declarations(declarations) {}
    std::list<std::shared_ptr<AstNode>> get_declarations() { return this->declarations; }
    void accept(AstNodeVisitor* visitor);
};

class Assignment : public Expression {
   private:
    std::shared_ptr<AstNode> target;
    std::shared_ptr<Expression> expr;
   public:
    Assignment(std::shared_ptr<AstNode> target, std::shared_ptr<Expression> expr) : target(target), expr(expr) {}
    std::shared_ptr<AstNode> get_target() { return this->target; }
    std::shared_ptr<Expression> get_expr() { return this->expr; }
    void accept(AstNodeVisitor* visitor);
};

class Binary : public Expression {
   private:
    Token op;
    std::shared_ptr<Expression> lhs;
    std::shared_ptr<Expression> rhs;
   public:
    Binary(Token op, std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs) : op(op), lhs(lhs), rhs(rhs) {}
    Token get_op() { return this->op; }
    std::shared_ptr<Expression> get_lhs() { return this->lhs; }
    std::shared_ptr<Expression> get_rhs() { return this->rhs; }
    void accept(AstNodeVisitor* visitor);
};

class Unary : public Expression {
   private:
    Token op;
    std::shared_ptr<Expression> operand;
   public:
    Unary(Token op, std::shared_ptr<Expression> operand) : op(op), operand(operand) {}
    Token get_op() { return this->op; }
    std::shared_ptr<Expression> get_operand() { return this->operand; }
    void accept(AstNodeVisitor* visitor);
};

class Variable : public Expression {
   private:
    Token name;
   public:
    Variable(Token name) : name(name) {}
    Token get_name() { return this->name; }
    void accept(AstNodeVisitor* visitor);
};

class Literal : public Expression {
   private:
    Token value;
   public:
    Literal(Token value) : value(value) {}
    Token get_value() { return this->value; }
    void accept(AstNodeVisitor* visitor);
};

class String : public Expression {
   private:
    Token value;
   public:
    String(Token value) : value(value) {}
    Token get_value() { return this->value; }
    void accept(AstNodeVisitor* visitor);
};

class Number : public Expression {
   private:
    Token value;
   public:
    Number(Token value) : value(value) {}
    Token get_value() { return this->value; }
    void accept(AstNodeVisitor* visitor);
};

class Call : public Expression {
   private:
    Token fname;
    std::list<std::shared_ptr<Expression>> arguments;
   public:
    Call(Token fname, std::list<std::shared_ptr<Expression>> arguments) : fname(fname), arguments(arguments) {}
    Token get_fname() { return this->fname; }
    std::list<std::shared_ptr<Expression>> get_arguments() { return this->arguments; }
    void accept(AstNodeVisitor* visitor);
};

class AstNodeVisitor {
   public:
    virtual void visit_statement(Statement* statement) { assert(false && "visit statement not overwritten"); };
    virtual void visit_expression(Expression* expression) { assert(false && "visit expression not overwritten"); };
    virtual void visit_type(Type* type) { assert(false && "visit type not overwritten"); };
    virtual void visit_formalparam(FormalParam* formalparam) { assert(false && "visit formalparam not overwritten"); };
    virtual void visit_fndecl(FnDecl* fndecl) { assert(false && "visit fndecl not overwritten"); };
    virtual void visit_letdecl(LetDecl* letdecl) { assert(false && "visit letdecl not overwritten"); };
    virtual void visit_exprstmt(ExprStmt* exprstmt) { assert(false && "visit exprstmt not overwritten"); };
    virtual void visit_returnstmt(ReturnStmt* returnstmt) { assert(false && "visit returnstmt not overwritten"); };
    virtual void visit_whilestmt(WhileStmt* whilestmt) { assert(false && "visit whilestmt not overwritten"); };
    virtual void visit_block(Block* block) { assert(false && "visit block not overwritten"); };
    virtual void visit_assignment(Assignment* assignment) { assert(false && "visit assignment not overwritten"); };
    virtual void visit_binary(Binary* binary) { assert(false && "visit binary not overwritten"); };
    virtual void visit_unary(Unary* unary) { assert(false && "visit unary not overwritten"); };
    virtual void visit_variable(Variable* variable) { assert(false && "visit variable not overwritten"); };
    virtual void visit_literal(Literal* literal) { assert(false && "visit literal not overwritten"); };
    virtual void visit_string(String* string) { assert(false && "visit string not overwritten"); };
    virtual void visit_number(Number* number) { assert(false && "visit number not overwritten"); };
    virtual void visit_call(Call* call) { assert(false && "visit call not overwritten"); };
};

} // namespace carl
#endif