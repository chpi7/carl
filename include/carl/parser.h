#ifndef carl_parser_h
#define carl_parser_h

#include <memory>
#include <vector>

#include "carl/ast/ast.h"
#include "carl/scanner.h"

namespace carl {

enum Precedence {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQ,
    PREC_COMP,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY,
};

class Parser;
// https://websites.umich.edu/~eecs381/handouts/Pointers_to_memberfuncs.pdf
using ParseFn = std::shared_ptr<AstNode> (Parser::*)();

struct ParseRule {
    Precedence prec;
    ParseFn prefix;
    ParseFn infix;
};

class Parser {
   private:
    bool panic_mode;
    bool has_error;
    std::shared_ptr<Scanner> scanner;
    Token current;
    Token previous;

   public:
    Parser();
    void set_scanner(std::shared_ptr<Scanner> scanner);
    std::vector<std::shared_ptr<AstNode>> parse();
    std::shared_ptr<AstNode> declaration();
    std::shared_ptr<AstNode> statement();
    std::shared_ptr<AstNode> expr_stmt();
    std::shared_ptr<AstNode> let_stmt();
    std::shared_ptr<AstNode> expression();
    std::shared_ptr<AstNode> unary();
    std::shared_ptr<AstNode> literal();
    std::shared_ptr<AstNode> variable();
    std::shared_ptr<AstNode> string();
    std::shared_ptr<AstNode> number();
    std::shared_ptr<AstNode> binary();
    std::shared_ptr<AstNode> assignment();
    std::shared_ptr<AstNode> grouping();

   private:
    void error_at(Token token, const char* message);
    void consume(TokenType type, const char* message);
    void advance();
    void synchronize();
    bool match(TokenType tokenType);
    bool peek(TokenType tokenType);
    std::shared_ptr<AstNode> parse_precedence(Precedence precedence);
    const ParseRule* get_rule(TokenType tokenType) const;
};

}  // namespace carl
#endif