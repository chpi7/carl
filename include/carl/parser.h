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
using ParseFn = std::shared_ptr<Expression> (Parser::*)();

struct ParseRule {
    Precedence prec;
    ParseFn prefix;
    ParseFn infix;
};

class Parser {
   private:
    bool panic_mode;
    std::shared_ptr<Scanner> scanner;
    Token current;
    Token previous;

   public:
    bool has_error;

    Parser();
    void set_scanner(std::shared_ptr<Scanner> scanner);

    std::vector<std::shared_ptr<AstNode>> parse();
    std::shared_ptr<AstNode> declaration();
    std::shared_ptr<LetDecl> let_decl();
    std::shared_ptr<Type> type();
    std::shared_ptr<FnDecl> fn_decl();

    std::shared_ptr<Statement> statement();
    std::shared_ptr<ReturnStmt> ret_stmt();
    std::shared_ptr<ExprStmt> expr_stmt();
    std::shared_ptr<WhileStmt> while_stmt();
    std::shared_ptr<Block> block();

    std::shared_ptr<Expression> expression();
    std::shared_ptr<Expression> grouping();
    std::shared_ptr<Call> call();
    // can be Assignment or Binary
    std::shared_ptr<Expression> binary();
    std::shared_ptr<Expression> unary();
    std::shared_ptr<Expression> string();
    std::shared_ptr<Expression> number();
    std::shared_ptr<Expression> variable();
    std::shared_ptr<Expression> literal();

   private:
    void error_at(Token token, const char* message);
    void consume(TokenType type, const char* message);
    void advance();
    void synchronize();
    bool match(TokenType tokenType);
    bool peek(TokenType tokenType);
    bool peek_next(TokenType tokenType);
    std::shared_ptr<Expression> parse_precedence(Precedence precedence);
    const ParseRule* get_rule(TokenType tokenType) const;
};

}  // namespace carl
#endif