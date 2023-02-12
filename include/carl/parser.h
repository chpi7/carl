#ifndef carl_parser_h
#define carl_parser_h

#include <memory>

#include "carl/ast.h"
#include "carl/scanner.h"

namespace carl {

enum Precedence {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
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
    Parser(std::shared_ptr<Scanner> scanner);
    std::shared_ptr<AstNode> expression();
    std::shared_ptr<AstNode> unary();
    std::shared_ptr<AstNode> primary();
    std::shared_ptr<AstNode> binary();

   private:
    void error_at(Token token, const char* message);
    void consume(TokenType type, const char* message);
    void advance();
    bool match(TokenType tokenType);
    std::shared_ptr<AstNode> parse_precedence(Precedence precedence);
    const ParseRule* get_rule(TokenType tokenType) const;
};

}  // namespace carl
#endif