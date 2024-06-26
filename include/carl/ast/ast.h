#ifndef carl_ast_h
#define carl_ast_h

#include <sstream>
#include <fstream>
#include <memory>
#include <list>
#include <vector>
#include <string>
#include "carl/scanner.h"
#include "carl/common.h"
#include "carl/ast/types.h"

namespace carl {

class AstNodeVisitor;
class Variable;

enum class AstNodeType { Statement, Block, Expression, Type, FormalParam, FnDecl, LetDecl, AdtStmt, ExprStmt, ReturnStmt, WhileStmt, Assignment, Binary, Unary, Variable, Literal, String, Number, Call, MatchArm, Match };

class AstNode {
   public:
    virtual ~AstNode() = default;
    virtual void accept(AstNodeVisitor* visitor) = 0;
    virtual AstNodeType get_node_type() const = 0;
};

class Statement : public AstNode {
   public:
    AstNodeType get_node_type() const;
    Statement() {}
    void accept(AstNodeVisitor* visitor);
};

class Block : public Statement {
   private:
    std::list<std::shared_ptr<AstNode>> declarations;
   public:
    AstNodeType get_node_type() const;
    Block(std::list<std::shared_ptr<AstNode>> declarations) : declarations(declarations) {}
    const std::list<std::shared_ptr<AstNode>>& get_declarations() const { return this->declarations; }
    void accept(AstNodeVisitor* visitor);
};

class Expression : public AstNode {
   private:
    std::shared_ptr<types::Type> type;
   public:
    AstNodeType get_node_type() const;
    Expression() {
        this->type = std::make_shared<types::Unknown>();
    }
    std::shared_ptr<types::Type> get_type() const { return this->type; }
    void set_type(std::shared_ptr<types::Type> type) { this->type = type;}
    void accept(AstNodeVisitor* visitor);
};

class Type : public AstNode {
   private:
    Token name;
   public:
    AstNodeType get_node_type() const;
    Type(Token name) : name(name) {}
    const Token& get_name() const { return this->name; }
    void accept(AstNodeVisitor* visitor);
};

class FormalParam : public AstNode {
   private:
    Token name;
    std::shared_ptr<types::Type> type;
   public:
    AstNodeType get_node_type() const;
    FormalParam(Token name) : name(name) {
        this->type = std::make_shared<types::Unknown>();
    }
    const Token& get_name() const { return this->name; }
    std::shared_ptr<types::Type> get_type() const { return this->type; }
    void set_type(std::shared_ptr<types::Type> type) { this->type = type;}
    void accept(AstNodeVisitor* visitor);
};

class FnDecl : public AstNode {
   private:
    Token name;
    std::string sname;
    std::list<std::shared_ptr<FormalParam>> formals;
    std::shared_ptr<Block> body;
    std::shared_ptr<types::Type> type;
    std::list<std::shared_ptr<Variable>> captures;
    bool is_extern;
   public:
    AstNodeType get_node_type() const;
    FnDecl(Token name, std::list<std::shared_ptr<FormalParam>> formals, std::shared_ptr<Block> body) : name(name), formals(formals), body(body) {
        this->sname = std::string(name.start,name.length);
        this->type = std::make_shared<types::Unknown>();
        this->captures = std::list<std::shared_ptr<Variable>>();
        this->is_extern = false;
    }
    const Token& get_name() const { return this->name; }
    const std::string& get_sname() const { return this->sname; }
    const std::list<std::shared_ptr<FormalParam>>& get_formals() const { return this->formals; }
    std::shared_ptr<Block> get_body() const { return this->body; }
    std::shared_ptr<types::Type> get_type() const { return this->type; }
    const std::list<std::shared_ptr<Variable>>& get_captures() const { return this->captures; }
    const bool& get_is_extern() const { return this->is_extern; }
    void set_sname(std::string sname) { this->sname = sname;}
    void set_type(std::shared_ptr<types::Type> type) { this->type = type;}
    void set_captures(std::list<std::shared_ptr<Variable>> captures) { this->captures = captures;}
    void set_is_extern(bool is_extern) { this->is_extern = is_extern;}
    void accept(AstNodeVisitor* visitor);
};

class LetDecl : public AstNode {
   private:
    Token name;
    std::shared_ptr<Expression> initializer;
    std::shared_ptr<types::Type> type;
   public:
    AstNodeType get_node_type() const;
    LetDecl(Token name, std::shared_ptr<Expression> initializer) : name(name), initializer(initializer) {
        this->type = std::make_shared<types::Unknown>();
    }
    const Token& get_name() const { return this->name; }
    std::shared_ptr<Expression> get_initializer() const { return this->initializer; }
    std::shared_ptr<types::Type> get_type() const { return this->type; }
    void set_type(std::shared_ptr<types::Type> type) { this->type = type;}
    void accept(AstNodeVisitor* visitor);
};

class AdtStmt : public Statement {
   private:
    std::shared_ptr<types::Type> type;
   public:
    AstNodeType get_node_type() const;
    AdtStmt(std::shared_ptr<types::Type> type) : type(type) {}
    std::shared_ptr<types::Type> get_type() const { return this->type; }
    void accept(AstNodeVisitor* visitor);
};

class ExprStmt : public Statement {
   private:
    std::shared_ptr<Expression> expr;
   public:
    AstNodeType get_node_type() const;
    ExprStmt(std::shared_ptr<Expression> expr) : expr(expr) {}
    std::shared_ptr<Expression> get_expr() const { return this->expr; }
    void accept(AstNodeVisitor* visitor);
};

class ReturnStmt : public Statement {
   private:
    std::shared_ptr<Expression> expr;
   public:
    AstNodeType get_node_type() const;
    ReturnStmt(std::shared_ptr<Expression> expr) : expr(expr) {}
    std::shared_ptr<Expression> get_expr() const { return this->expr; }
    void accept(AstNodeVisitor* visitor);
};

class WhileStmt : public Statement {
   private:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> body;
   public:
    AstNodeType get_node_type() const;
    WhileStmt(std::shared_ptr<Expression> condition, std::shared_ptr<Statement> body) : condition(condition), body(body) {}
    std::shared_ptr<Expression> get_condition() const { return this->condition; }
    std::shared_ptr<Statement> get_body() const { return this->body; }
    void accept(AstNodeVisitor* visitor);
};

class Assignment : public Expression {
   private:
    std::shared_ptr<Expression> target;
    std::shared_ptr<Expression> expr;
   public:
    AstNodeType get_node_type() const;
    Assignment(std::shared_ptr<Expression> target, std::shared_ptr<Expression> expr) : target(target), expr(expr) {}
    std::shared_ptr<Expression> get_target() const { return this->target; }
    std::shared_ptr<Expression> get_expr() const { return this->expr; }
    void accept(AstNodeVisitor* visitor);
};

class Binary : public Expression {
   private:
    Token op;
    std::shared_ptr<Expression> lhs;
    std::shared_ptr<Expression> rhs;
   public:
    AstNodeType get_node_type() const;
    Binary(Token op, std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs) : op(op), lhs(lhs), rhs(rhs) {}
    const Token& get_op() const { return this->op; }
    std::shared_ptr<Expression> get_lhs() const { return this->lhs; }
    std::shared_ptr<Expression> get_rhs() const { return this->rhs; }
    void accept(AstNodeVisitor* visitor);
};

class Unary : public Expression {
   private:
    Token op;
    std::shared_ptr<Expression> operand;
   public:
    AstNodeType get_node_type() const;
    Unary(Token op, std::shared_ptr<Expression> operand) : op(op), operand(operand) {}
    const Token& get_op() const { return this->op; }
    std::shared_ptr<Expression> get_operand() const { return this->operand; }
    void accept(AstNodeVisitor* visitor);
};

class Variable : public Expression {
   private:
    Token name;
   public:
    AstNodeType get_node_type() const;
    Variable(Token name) : name(name) {}
    const Token& get_name() const { return this->name; }
    void accept(AstNodeVisitor* visitor);
};

class Literal : public Expression {
   private:
    Token value;
   public:
    AstNodeType get_node_type() const;
    Literal(Token value) : value(value) {}
    const Token& get_value() const { return this->value; }
    void accept(AstNodeVisitor* visitor);
};

class String : public Expression {
   private:
    Token value;
   public:
    AstNodeType get_node_type() const;
    String(Token value) : value(value) {}
    const Token& get_value() const { return this->value; }
    void accept(AstNodeVisitor* visitor);
};

class Number : public Expression {
   private:
    Token value;
   public:
    AstNodeType get_node_type() const;
    Number(Token value) : value(value) {}
    const Token& get_value() const { return this->value; }
    void accept(AstNodeVisitor* visitor);
};

class Call : public Expression {
   private:
    Token fname;
    std::list<std::shared_ptr<Expression>> arguments;
   public:
    AstNodeType get_node_type() const;
    Call(Token fname, std::list<std::shared_ptr<Expression>> arguments) : fname(fname), arguments(arguments) {}
    const Token& get_fname() const { return this->fname; }
    const std::list<std::shared_ptr<Expression>>& get_arguments() const { return this->arguments; }
    void accept(AstNodeVisitor* visitor);
};

class MatchArm : public Expression {
   private:
    std::shared_ptr<Expression> result;
   public:
    AstNodeType get_node_type() const;
    MatchArm(std::shared_ptr<Expression> result) : result(result) {}
    std::shared_ptr<Expression> get_result() const { return this->result; }
    void accept(AstNodeVisitor* visitor);
};

class Match : public Expression {
   private:
    std::shared_ptr<Expression> matchee;
    std::list<std::shared_ptr<MatchArm>> arms;
   public:
    AstNodeType get_node_type() const;
    Match(std::shared_ptr<Expression> matchee, std::list<std::shared_ptr<MatchArm>> arms) : matchee(matchee), arms(arms) {}
    std::shared_ptr<Expression> get_matchee() const { return this->matchee; }
    const std::list<std::shared_ptr<MatchArm>>& get_arms() const { return this->arms; }
    void accept(AstNodeVisitor* visitor);
};

class AstNodeVisitor {
   public:
    virtual void visit_statement(Statement* statement) { assert(false && "visit statement not overwritten"); };
    virtual void visit_block(Block* block) { assert(false && "visit block not overwritten"); };
    virtual void visit_expression(Expression* expression) { assert(false && "visit expression not overwritten"); };
    virtual void visit_type(Type* type) { assert(false && "visit type not overwritten"); };
    virtual void visit_formalparam(FormalParam* formalparam) { assert(false && "visit formalparam not overwritten"); };
    virtual void visit_fndecl(FnDecl* fndecl) { assert(false && "visit fndecl not overwritten"); };
    virtual void visit_letdecl(LetDecl* letdecl) { assert(false && "visit letdecl not overwritten"); };
    virtual void visit_adtstmt(AdtStmt* adtstmt) { assert(false && "visit adtstmt not overwritten"); };
    virtual void visit_exprstmt(ExprStmt* exprstmt) { assert(false && "visit exprstmt not overwritten"); };
    virtual void visit_returnstmt(ReturnStmt* returnstmt) { assert(false && "visit returnstmt not overwritten"); };
    virtual void visit_whilestmt(WhileStmt* whilestmt) { assert(false && "visit whilestmt not overwritten"); };
    virtual void visit_assignment(Assignment* assignment) { assert(false && "visit assignment not overwritten"); };
    virtual void visit_binary(Binary* binary) { assert(false && "visit binary not overwritten"); };
    virtual void visit_unary(Unary* unary) { assert(false && "visit unary not overwritten"); };
    virtual void visit_variable(Variable* variable) { assert(false && "visit variable not overwritten"); };
    virtual void visit_literal(Literal* literal) { assert(false && "visit literal not overwritten"); };
    virtual void visit_string(String* string) { assert(false && "visit string not overwritten"); };
    virtual void visit_number(Number* number) { assert(false && "visit number not overwritten"); };
    virtual void visit_call(Call* call) { assert(false && "visit call not overwritten"); };
    virtual void visit_matcharm(MatchArm* matcharm) { assert(false && "visit matcharm not overwritten"); };
    virtual void visit_match(Match* match) { assert(false && "visit match not overwritten"); };
};

} // namespace carl
#endif