#ifndef carl_ast_h
#define carl_ast_h

#include <sstream>
#include <fstream>
#include <memory>

#include "carl/scanner.h"

namespace carl {

class AstNodeVisitor;

class AstNode {
   public:
    virtual ~AstNode() = default;
    virtual void accept(AstNodeVisitor* visitor) { printf("test!"); }
};

class Binary : public AstNode {
   private:
    Token op;
    std::shared_ptr<AstNode> lhs;
    std::shared_ptr<AstNode> rhs;

   public:
    Binary(Token op, std::shared_ptr<AstNode> lhs, std::shared_ptr<AstNode> rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)){};
    Token* get_op() { return &this->op; }
    std::shared_ptr<AstNode> get_lhs() { return this->lhs; };
    std::shared_ptr<AstNode> get_rhs() { return this->rhs; };
    void accept(AstNodeVisitor* visitor);
};

class Unary : public AstNode {
   private:
    Token op;
    std::shared_ptr<AstNode> value;

   public:
    Unary(Token op, std::shared_ptr<AstNode> value)
        : op(op), value(std::move(value)){};
    Token* get_op() { return &this->op; }
    std::shared_ptr<AstNode> get_value() { return this->value; };
    void accept(AstNodeVisitor* visitor);
};

class Variable : public AstNode {
   private:
    Token name;

   public:
    Variable(Token name) : name(name){};
    Token* get_name() { return &this->name; }
    void accept(AstNodeVisitor* visitor);
};

class Literal : public AstNode {
   private:
    Token value;

   public:
    Literal(Token value) : value(value){};
    Token* get_value() { return &this->value; }
    void accept(AstNodeVisitor* visitor);
};

class AstNodeVisitor {
   public:
    virtual void visit_binary(Binary* node) = 0;
    virtual void visit_unary(Unary* node) = 0;
    virtual void visit_variable(Variable* node) = 0;
    virtual void visit_literal(Literal* node) = 0;
};

class PrintAstNodeVisitor : public AstNodeVisitor {
   private:
    std::ostringstream& os;

   public:
    PrintAstNodeVisitor(std::ostringstream& os) : os(os) {};
    void visit_binary(Binary* node);
    void visit_unary(Unary* node);
    void visit_variable(Variable* node);
    void visit_literal(Literal* node);
};

}  // namespace carl
#endif