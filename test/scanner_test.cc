#include "carl/scanner.h"

#include <gtest/gtest.h>

#include <vector>

using namespace carl;

namespace {

TEST(Scanner, scanSingle) {
    Scanner scanner;
    auto test = "(){},.-+;/*|||&&&";
    scanner.init(test);

    std::vector<TokenType> tokens;
    Token t;
    do {
        t = scanner.scan_token();
        tokens.push_back(t.type);
    } while (t.type != TOKEN_EOF);

    std::vector<TokenType> expected = {
        TOKEN_LEFT_PAREN,  TOKEN_RIGHT_PAREN, TOKEN_LEFT_BRACE,
        TOKEN_RIGHT_BRACE, TOKEN_COMMA,       TOKEN_DOT,
        TOKEN_MINUS,       TOKEN_PLUS,        TOKEN_SEMICOLON,
        TOKEN_SLASH,       TOKEN_STAR,        TOKEN_OR,
        TOKEN_PIPE,        TOKEN_AND,         TOKEN_ERROR,
        TOKEN_EOF};

    ASSERT_EQ(tokens, expected);
}

TEST(Scanner, scanManyTokens) {
    Scanner scanner;
    auto test =
        "1 + 2.3 hey { _ ) \"this is a let if >= = string\" _abcdef_gf1234_h "
        "let for if - //blubb this is a comment\n*; !test/ <= == !=>=fn";
    scanner.init(test);

    std::vector<Token> tokens;
    Token t;
    do {
        t = scanner.scan_token();
        tokens.push_back(t);
    } while (t.type != TOKEN_EOF);

    std::vector<TokenType> expected = {
        TOKEN_NUMBER,        TOKEN_PLUS,        TOKEN_NUMBER,
        TOKEN_IDENTIFIER,    TOKEN_LEFT_BRACE,  TOKEN_UNDERSCORE,
        TOKEN_RIGHT_PAREN,   TOKEN_STRING,      TOKEN_IDENTIFIER,
        TOKEN_LET,           TOKEN_FOR,         TOKEN_IF,
        TOKEN_MINUS,         TOKEN_STAR,        TOKEN_SEMICOLON,
        TOKEN_BANG,          TOKEN_IDENTIFIER,  TOKEN_SLASH,
        TOKEN_LESS_EQUAL,    TOKEN_EQUAL_EQUAL, TOKEN_BANG_EQUAL,
        TOKEN_GREATER_EQUAL, TOKEN_FN,          TOKEN_EOF};

    ASSERT_EQ(tokens.size(), expected.size());
    for (int i = 0; i < tokens.size(); ++i) {
        ASSERT_EQ(tokens[i].type, expected[i]);
    }
}
}  // namespace