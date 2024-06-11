#pragma once

#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "carl/ast/ast.h"
#include "carl/ast/types.h"
#include "carl/name_environment.h"
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
    PREC_COMPOSITION,
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

struct ParseError {
    std::string message;
};

using ParseResult = Result<std::vector<std::shared_ptr<AstNode>>, ParseError>;

class Parser {
   private:
    bool panic_mode;
    std::shared_ptr<Scanner> scanner;
    std::unique_ptr<Environment<Variable*>> environment;
    std::unique_ptr<Environment<FnDecl*>> fn_environment;
    std::vector<std::string> known_adt_constructor_names;
    // whats the function we are currently in
    std::vector<int> current_fn_env_id;
    std::vector<std::list<std::shared_ptr<Variable>>> captured_variables;

    Token current;
    Token previous;

   public:
    bool has_error;

    Parser();
    void set_scanner(std::shared_ptr<Scanner> scanner);

    ParseResult parse_r(std::string& src, bool add_builtins = true, bool skip_type_checking = false);
    std::vector<std::shared_ptr<AstNode>> parse();
    std::vector<std::shared_ptr<AstNode>> parse(std::string& src);
    std::shared_ptr<AstNode> declaration();
    std::shared_ptr<LetDecl> let_decl();
    std::shared_ptr<types::Type> type();
    std::shared_ptr<FnDecl> fn_decl();

    std::shared_ptr<Statement> statement();
    std::shared_ptr<ReturnStmt> ret_stmt();
    std::shared_ptr<ExprStmt> expr_stmt();
    std::shared_ptr<WhileStmt> while_stmt();
    std::shared_ptr<Block> block();
    std::shared_ptr<AdtStmt> adt_stmt();

    std::shared_ptr<Expression> expression();
    std::shared_ptr<Expression> grouping();
    std::shared_ptr<Expression> match();
    std::shared_ptr<Expression> call();
    // can be Assignment or Binary
    std::shared_ptr<Expression> binary();
    std::shared_ptr<Expression> unary();
    std::shared_ptr<Expression> string();
    std::shared_ptr<Expression> number();
    std::shared_ptr<Expression> variable();
    std::shared_ptr<Expression> literal();

   private:
    bool is_captured(const std::string& name);
    bool is_known_name(const std::string& name);
    void error_at(Token token, const char* message);
    void consume(TokenType type, const char* message);
    void advance();
    void synchronize();
    bool match(TokenType tokenType);
    bool peek(TokenType tokenType);
    bool peek_next(TokenType tokenType);
    std::shared_ptr<Expression> parse_precedence(Precedence precedence);
    const ParseRule* get_rule(TokenType tokenType) const;
    std::shared_ptr<FnDecl> decl_builtin(
        const std::string& name,
        std::vector<std::shared_ptr<types::Type>> param_types,
        std::shared_ptr<types::Type> return_type);
    std::shared_ptr<types::Type> parse_data_definition(std::string& name);

    types::Adt::Constructor parse_adt_constructor();
};
}  // namespace carl
