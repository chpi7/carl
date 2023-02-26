#include "carl/parser.h"

#include <unordered_map>

#include "carl/common.h"

/* Grammar Excerpt

    program -> declaration*
    declaration -> letDecl | fnDecl | statement

    statement -> ifStatement | whileStatment | blockStatement |
   expressionStatement ifStatement -> 'if' expression statement [else statement]
    whileStatement -> 'while' expression statement
    blockStatement -> { declaration* }
    expressionStatement -> statement ';'

    expression -> assignment    // maybe support if expression here
    assignment -> ...           // according to operator precedence (= is right
   associative, left associative otherwise)
*/

namespace carl {

static std::shared_ptr<AstNode> make_error_node() {
    return std::make_shared<Invalid>();
}

static const std::unordered_map<TokenType, ParseRule> parse_rules = {
    {TOKEN_LEFT_PAREN, {PREC_NONE, &Parser::grouping, nullptr}},
    {TOKEN_RIGHT_PAREN, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_LEFT_BRACE, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_RIGHT_BRACE, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_COMMA, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_DOT, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_MINUS, {PREC_TERM, &Parser::unary, &Parser::binary}},
    {TOKEN_PLUS, {PREC_TERM, nullptr, &Parser::binary}},
    {TOKEN_SEMICOLON, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_SLASH, {PREC_FACTOR, nullptr, &Parser::binary}},
    {TOKEN_STAR, {PREC_FACTOR, nullptr, &Parser::binary}},
    {TOKEN_PIPE, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_PERC, {PREC_FACTOR, nullptr, &Parser::binary}},
    {TOKEN_BANG, {PREC_NONE, &Parser::unary, nullptr}},
    {TOKEN_BANG_EQUAL, {PREC_EQ, nullptr, &Parser::binary}},
    {TOKEN_EQUAL, {PREC_ASSIGNMENT, nullptr, &Parser::binary}},
    {TOKEN_EQUAL_EQUAL, {PREC_EQ, nullptr, &Parser::binary}},
    {TOKEN_GREATER, {PREC_COMP, nullptr, &Parser::binary}},
    {TOKEN_GREATER_EQUAL, {PREC_COMP, nullptr, &Parser::binary}},
    {TOKEN_LESS, {PREC_COMP, nullptr, &Parser::binary}},
    {TOKEN_LESS_EQUAL, {PREC_COMP, nullptr, &Parser::binary}},
    {TOKEN_AND, {PREC_AND, nullptr, &Parser::binary}},
    {TOKEN_OR, {PREC_OR, nullptr, &Parser::binary}},
    {TOKEN_IDENTIFIER, {PREC_NONE, &Parser::variable, nullptr}},
    {TOKEN_STRING, {PREC_NONE, &Parser::string, nullptr}},
    {TOKEN_NUMBER, {PREC_NONE, &Parser::number, nullptr}},
    {TOKEN_IF, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_ELSE, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_FOR, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_FN, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_TRUE, {PREC_NONE, &Parser::literal, nullptr}},
    {TOKEN_FALSE, {PREC_NONE, &Parser::literal, nullptr}},
    {TOKEN_NIL, {PREC_NONE, &Parser::literal, nullptr}},
    {TOKEN_RETURN, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_LET, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_ERROR, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_EOF, {PREC_NONE, nullptr, nullptr}},
};

std::shared_ptr<AstNode> Parser::parse_precedence(Precedence precedence) {
    auto prefix_rule = get_rule(current.type);
    // assert(get_rule(current.type)->prefix != nullptr && "No prefix rule for
    // token");
    if (prefix_rule->prefix == nullptr) {
        error_at(current, "no prefix rule found.");
        return make_error_node();
    }
    // so nice :)
    std::shared_ptr<AstNode> expression = (this->*(prefix_rule->prefix))();

    const ParseRule* infix_rule;
    while (precedence <= (infix_rule = get_rule(current.type))->prec) {
        assert(get_rule(current.type)->infix != nullptr &&
               "No infix rule for token");

        auto op_token = current;
        advance();

        if (op_token.type == TOKEN_EQUAL) {
            if (precedence > PREC_ASSIGNMENT) {
                error_at(current, "assignment no possible here");
                return make_error_node();
            } else {
                // assignment
                auto value = (this->*(infix_rule)->infix)();
                expression = std::make_shared<Assignment>(expression, value);
            }
        } else {
            // normal binop
            auto rhs = (this->*(infix_rule)->infix)();
            expression = std::make_shared<Binary>(op_token, expression, rhs);
        }
    }

    return expression;
}

Parser::Parser() : scanner(nullptr) {
    panic_mode = false;
    has_error = false;
}

void Parser::set_scanner(std::shared_ptr<Scanner> scanner) {
    this->scanner = scanner;
    advance();
}

const ParseRule* Parser::get_rule(TokenType tokenType) const {
    if (!parse_rules.contains(tokenType)) {
        return nullptr;
    }
    auto c = &parse_rules.at(tokenType);
    return c;
}

void Parser::synchronize() {
    panic_mode = false;

    while (current.type != TOKEN_EOF) {
        if (previous.type == TOKEN_SEMICOLON) return;
        switch (current.type) {
            case TOKEN_FN:
            case TOKEN_LET:
            case TOKEN_IF:
            case TOKEN_RETURN:
                return;
            default:;
        }
        advance();
    }
}

std::vector<std::shared_ptr<AstNode>> Parser::parse() {
    std::vector<std::shared_ptr<AstNode>> result;
    while (current.type != TOKEN_EOF) {
        result.push_back(declaration());
        if (panic_mode) {
            synchronize();
        }
    }
    return result;
}

std::shared_ptr<AstNode> Parser::declaration() {
    if (match(TOKEN_LET)) {
        return let_decl();
    } else {
        return statement();
    }
}

std::shared_ptr<AstNode> Parser::statement() {
    return expr_stmt();
}

std::shared_ptr<AstNode> Parser::expr_stmt() {
    auto expr = expression();
    consume(TOKEN_SEMICOLON, "Expected ';' at the end of statement.");
    return std::make_shared<ExprStmt>(expr);
}

std::shared_ptr<AstNode> Parser::let_decl() {
    consume(TOKEN_IDENTIFIER,
            "Expected identifier as variable name after let.");
    auto identifier = previous;
    if (peek(TOKEN_SEMICOLON)) {
        error_at(current,
                 "Expected initialization after variable declaration.");
        return make_error_node();
    }
    if (match(TOKEN_EQUAL)) {
        auto initializer = expression();
        auto result = std::make_shared<LetStmt>(identifier, initializer);
        if (!match(TOKEN_SEMICOLON)) {
            error_at(current, "Expected ; after let initializer.");
        }
        return result;
    } else {
        error_at(current, "Expected ';' or '=' after 'let _'.");
        return make_error_node();
    }
}

std::shared_ptr<AstNode> Parser::expression() {
    return parse_precedence(PREC_ASSIGNMENT);
}

std::shared_ptr<AstNode> Parser::literal() {
    advance();
    return std::make_shared<Literal>(previous);
}

std::shared_ptr<AstNode> Parser::number() {
    advance();
    return std::make_shared<Number>(previous);
}

std::shared_ptr<AstNode> Parser::string() {
    advance();
    return std::make_shared<String>(previous);
}

std::shared_ptr<AstNode> Parser::unary() {
    advance();  // Consume the unary operator.
    auto op_token = previous;
    return std::make_shared<Unary>(op_token, parse_precedence(PREC_UNARY));
}

std::shared_ptr<AstNode> Parser::variable() {
    consume(TOKEN_IDENTIFIER, "Expected identifier as variable name.");
    return std::make_shared<Variable>(previous);
}

std::shared_ptr<AstNode> Parser::binary() {
    const ParseRule* current_rule = get_rule(previous.type);

    // small trick to make assignment right associative
    int prec_offset = 0;
    if (current_rule->prec != PREC_ASSIGNMENT) prec_offset++;

    return parse_precedence(
        static_cast<Precedence>(current_rule->prec + prec_offset));
}

std::shared_ptr<AstNode> Parser::grouping() {
    advance();
    auto contained_expression = expression();
    consume(TOKEN_RIGHT_PAREN,
            "Expected closing paren after grouping expression.");
    return contained_expression;
}

bool Parser::match(TokenType tokenType) {
    if (current.type != tokenType) return false;
    advance();
    return true;
}

bool Parser::peek(TokenType tokenType) {
    if (current.type != tokenType) return false;
    return true;
}

void Parser::consume(TokenType type, const char* message) {
    if (!match(type)) error_at(current, message);
}

void Parser::advance() {
    previous = current;
    current = scanner->scan_token();
}

void Parser::error_at(Token token, const char* message) {
    if (panic_mode) return;
    panic_mode = true;
    has_error = true;

    fprintf(stderr, "[line %d] Error", token.line);

    switch (token.type) {
        case TOKEN_EOF:
            fprintf(stderr, " at end");
            break;
        case TOKEN_ERROR:
            break;
        default:
            fprintf(stderr, " at '%.*s'", token.length, token.start);
            break;
    }

    fprintf(stderr, ": %s\n", message);
}
}  // namespace carl