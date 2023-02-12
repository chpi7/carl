#include "carl/parser.h"

#include <unordered_map>

namespace carl {

const std::unordered_map<TokenType, ParseRule> parse_rules = {
    {TOKEN_LEFT_PAREN, {PREC_NONE, nullptr, nullptr}},
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
    {TOKEN_BANG, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_BANG_EQUAL, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_EQUAL, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_EQUAL_EQUAL, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_GREATER, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_GREATER_EQUAL, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_LESS, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_LESS_EQUAL, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_IDENTIFIER, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_STRING, {PREC_NONE, &Parser::primary, nullptr}},
    {TOKEN_NUMBER, {PREC_NONE, &Parser::primary, nullptr}},
    {TOKEN_IF, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_ELSE, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_FOR, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_FN, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_TRUE, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_FALSE, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_NIL, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_RETURN, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_LET, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_ERROR, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_EOF, {PREC_NONE, nullptr, nullptr}},
};

Parser::Parser(std::shared_ptr<Scanner> scanner) : scanner(scanner) {
    panic_mode = false;
    has_error = false;
    advance();
}

const ParseRule* Parser::get_rule(TokenType tokenType) const {
    if (!parse_rules.contains(tokenType)) {
        return nullptr;
    }
    auto c = &parse_rules.at(tokenType);
    return c;
}

std::shared_ptr<AstNode> Parser::expression() {
    return parse_precedence(PREC_ASSIGNMENT);
}

std::shared_ptr<AstNode> Parser::primary() {
    if (match(TOKEN_NUMBER)) return std::make_shared<Literal>(previous);
    if (match(TOKEN_STRING)) return std::make_shared<Literal>(previous);
    // TODO: allow grouping here
    error_at(current, "Expected number or string as primary expression.");
    return std::make_shared<Literal>(current);
}

std::shared_ptr<AstNode> Parser::unary() {
    advance();  // Consume the unary operator.
    return std::make_shared<Unary>( previous, primary());
}

std::shared_ptr<AstNode> Parser::binary() {
    const ParseRule* current_rule = get_rule(previous.type);
    return parse_precedence(static_cast<Precedence>(current_rule->prec + 1));
}

bool Parser::match(TokenType tokenType) {
    if (current.type != tokenType) return false;
    advance();
    return true;
}

void Parser::consume(TokenType type, const char* message) {
    if (!match(type)) error_at(current, message);
}

void Parser::advance() {
    previous = current;
    current = scanner->scan_token();
}

std::shared_ptr<AstNode> Parser::parse_precedence(Precedence precedence) {
    auto prefix_rule = get_rule(current.type);
    // so nice :)
    std::shared_ptr<AstNode> expression = (this->*(prefix_rule->prefix))();

    const ParseRule* infix_rule;
    while (precedence <= (infix_rule = get_rule(current.type))->prec) {
        auto op_token = current;
        advance();

        auto rhs = (this->*(infix_rule)->infix)();
        expression = std::make_shared<Binary>(op_token, expression, rhs);
    }

    return expression;
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