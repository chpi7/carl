#ifndef carl_scanner_h
#define carl_scanner_h

#include <iostream>
#include <vector>

namespace carl {

enum TokenType {
    // single character tokens
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_SLASH,
    TOKEN_STAR,
    TOKEN_PIPE,
    TOKEN_PERC,

    // single and double char tokens
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_AND,
    TOKEN_OR,

    // literals
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,

    // keywords
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_FN,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NIL,
    TOKEN_RETURN,
    TOKEN_LET,

    TOKEN_ERROR,
    TOKEN_EOF,
};

struct Token {
    TokenType type;
    const char *start;
    int length;
    int line;
};

class Scanner {
   private:
    const char *start = nullptr;
    const char *current = nullptr;
    int line = 0;
    std::vector<Token> tokens;
    std::vector<Token>::iterator next_token;

   public:
    Scanner() = default;
    void init(const char *source);
    Token scan_token();
    Token peek_token();

   private:
    void scan_all();
    Token scan_token_internal();
    bool is_at_end();
    bool match(char expected);
    char advance();
    char peek();
    char peek_next();
    void skip_whitespace();
    TokenType check_keyword(int offset, int length, const char *expected,
                            TokenType type);
    TokenType get_identifier_type();
    Token identifier();
    Token string();
    Token number();
    Token make_error_token(const char *error_message);
    Token make_token(TokenType type);
};

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool is_digit(char c) { return c >= '0' && c <= '9'; }
}  // namespace carl

#endif