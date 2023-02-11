#include "carl/scanner.h"

#include <gtest/gtest.h>

#include <vector>

using namespace carl;

namespace {

TEST(Scanner, scanTokens1) {
    Scanner scanner;
    auto test =
        "1 + 2.3 hey { _ ) \"this is a let if >= = string\" abcdef_gf1234_h "
        "let for if - //blubb this is a comment\n*; !test/ <= == !=>=";
    scanner.init(test);

    std::vector<Token> tokens;
    Token t;
    do {
        t = scanner.scan_token();
        tokens.push_back(t);
    } while (t.type != TOKEN_EOF);

    std::vector<TokenType> expected = {
        TOKEN_NUMBER,        TOKEN_PLUS,        TOKEN_NUMBER,
        TOKEN_IDENTIFIER,    TOKEN_LEFT_BRACE,  TOKEN_IDENTIFIER,
        TOKEN_RIGHT_PAREN,   TOKEN_STRING,      TOKEN_IDENTIFIER,
        TOKEN_LET,           TOKEN_FOR,         TOKEN_IF,
        TOKEN_MINUS,         TOKEN_STAR,        TOKEN_SEMICOLON,
        TOKEN_BANG,          TOKEN_IDENTIFIER,  TOKEN_SLASH,
        TOKEN_LESS_EQUAL,    TOKEN_EQUAL_EQUAL, TOKEN_BANG_EQUAL,
        TOKEN_GREATER_EQUAL, TOKEN_EOF};

    ASSERT_EQ(tokens.size(), expected.size());
    for (int i = 0; i < tokens.size(); ++i) {
        ASSERT_EQ(tokens[i].type, expected[i]);
    }
}
}  // namespace