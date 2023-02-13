#ifndef carl_ast_h
#define carl_ast_h

#include <sstream>
#include <fstream>
#include <memory>
#include "carl/scanner.h"
#include "carl/common.h"

namespace carl {

class AstNodeVisitor;

class AstNode {
   public:
    virtual ~AstNode() = default;
    virtual void accept(AstNodeVisitor* visitor) = 0;
};

class Invalid : public AstNode {
   private:

   public:
    Invalid() {}

    void accept(AstNodeVisitor* visitor);
};

class ExprStmt : public AstNode {
   private:
    std::shared_ptr<AstNode> expr;
   public:
    ExprStmt(std::shared_ptr<AstNode> expr) :expr(expr) {}
    std::shared_ptr<AstNode> get_expr() { return this->expr; }
    void accept(AstNodeVisitor* visitor);
};

class LetStmt : public AstNode {
   private:
    Token name;
    std::shared_ptr<AstNode> initializer;
   public:
    LetStmt(Token name, std::shared_ptr<AstNode> initializer) :name(name), initializer(initializer) {}
    Token get_name() { return this->name; }
    std::shared_ptr<AstNode> get_initializer() { return this->initializer; }
    void accept(AstNodeVisitor* visitor);
};

class Binary : public AstNode {
   private:
    Token op;
    std::shared_ptr<AstNode> lhs;
    std::shared_ptr<AstNode> rhs;
   public:
    Binary(Token op, std::shared_ptr<AstNode> lhs, std::shared_ptr<AstNode> rhs) :op(op), lhs(lhs), rhs(rhs) {}
    Token get_op() { return this->op; }
    std::shared_ptr<AstNode> get_lhs() { return this->lhs; }
    std::shared_ptr<AstNode> get_rhs() { return this->rhs; }
    void accept(AstNodeVisitor* visitor);
};

class Unary : public AstNode {
   private:
    Token op;
    std::shared_ptr<AstNode> operand;
   public:
    Unary(Token op, std::shared_ptr<AstNode> operand) :op(op), operand(operand) {}
    Token get_op() { return this->op; }
    std::shared_ptr<AstNode> get_operand() { return this->operand; }
    void accept(AstNodeVisitor* visitor);
};

class Variable : public AstNode {
   private:
    Token name;
   public:
    Variable(Token name) :name(name) {}
    Token get_name() { return this->name; }
    void accept(AstNodeVisitor* visitor);
};

class Literal : public AstNode {
   private:
    Token value;
   public:
    Literal(Token value) :value(value) {}
    Token get_value() { return this->value; }
    void accept(AstNodeVisitor* visitor);
};

class String : public AstNode {
   private:
    Token value;
   public:
    String(Token value) :value(value) {}
    Token get_value() { return this->value; }
    void accept(AstNodeVisitor* visitor);
};

class Number : public AstNode {
   private:
    Token value;
   public:
    Number(Token value) :value(value) {}
    Token get_value() { return this->value; }
    void accept(AstNodeVisitor* visitor);
};

class AstNodeVisitor {
   public:
    virtual void visit_invalid(Invalid* invalid) { assert(false && "visit invalid not overwritten"); };
    virtual void visit_exprstmt(ExprStmt* exprstmt) { assert(false && "visit exprstmt not overwritten"); };
    virtual void visit_letstmt(LetStmt* letstmt) { assert(false && "visit letstmt not overwritten"); };
    virtual void visit_binary(Binary* binary) { assert(false && "visit binary not overwritten"); };
    virtual void visit_unary(Unary* unary) { assert(false && "visit unary not overwritten"); };
    virtual void visit_variable(Variable* variable) { assert(false && "visit variable not overwritten"); };
    virtual void visit_literal(Literal* literal) { assert(false && "visit literal not overwritten"); };
    virtual void visit_string(String* string) { assert(false && "visit string not overwritten"); };
    virtual void visit_number(Number* number) { assert(false && "visit number not overwritten"); };
};

} // namespace carl
#endif