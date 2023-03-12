#include "carl/scanner.h"

#include "memory.h"

namespace carl {

void Scanner::init(const char *source) {
    this->start = source;
    current = this->start;
    line = 1;

    // Eagerly tokenize the whole input for easy lookahead.
    scan_all();
    next_token = tokens.begin();
}

void Scanner::scan_all() {
    Token token;
    do {
        token = scan_token_internal();
        tokens.push_back(token);
    } while (token.type != TOKEN_EOF);
}

Token Scanner::scan_token() {
    if (next_token == tokens.end()) return tokens.back();
    return *(next_token++); 
}

Token Scanner::peek_token() {
    return *(next_token); 
}

Token Scanner::scan_token_internal() {
    skip_whitespace();
    start = current;

    if (is_at_end()) return make_token(TOKEN_EOF);

    char c = advance();

    if (is_alpha(c) || (c == '_' && (is_alpha_num(peek_next())))) return identifier();
    if (is_digit(c)) return number();
    if (c == '"') return string();

    switch (c) {
        case '(':
            return make_token(TOKEN_LEFT_PAREN);
        case ')':
            return make_token(TOKEN_RIGHT_PAREN);
        case '{':
            return make_token(TOKEN_LEFT_BRACE);
        case '}':
            return make_token(TOKEN_RIGHT_BRACE);
        case ',':
            return make_token(TOKEN_COMMA);
        case ';':
            return make_token(TOKEN_SEMICOLON);
        case ':':
            return make_token(TOKEN_COLON);
        case '.':
            return make_token(TOKEN_DOT);
        case '-':
            return make_token(TOKEN_MINUS);
        case '+':
            return make_token(TOKEN_PLUS);
        case '*':
            return make_token(TOKEN_STAR);
        case '/':
            return make_token(TOKEN_SLASH);
        case '%':
            return make_token(TOKEN_PERC);
        case '_':
            return make_token(TOKEN_UNDERSCORE);
        case '&':
            return match('&') ? make_token(TOKEN_AND) : make_token(TOKEN_ERROR);
        case '|':
            return match('|') ? make_token(TOKEN_OR) : make_token(TOKEN_PIPE);
        case '=':
            return match('=') ? make_token(TOKEN_EQUAL_EQUAL)
                       : make_token(TOKEN_EQUAL);
        case '!':
            return match('=') ? make_token(TOKEN_BANG_EQUAL) : make_token(TOKEN_BANG);
        case '<':
            return match('=') ? make_token(TOKEN_LESS_EQUAL) : make_token(TOKEN_LESS);
        case '>':
            return match('=') ? make_token(TOKEN_GREATER_EQUAL)
                       : make_token(TOKEN_GREATER);
        default:
            return make_token(TOKEN_ERROR);
    }
}

bool Scanner::is_at_end() { return *current == '\0'; }

bool Scanner::match(char expected) {
    if (is_at_end()) return false;
    if (peek() == expected) {
        advance();
        return true;
    } else {
        return false;
    }
}

char Scanner::advance() {
    current++;
    return current[-1];
}

char Scanner::peek() { return *current; }

char Scanner::peek_next() {
    if (is_at_end()) return '\0';
    return current[1];
}

void Scanner::skip_whitespace() {
    while (1) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                advance();
                break;
            case '\n':
                line++;
                advance();
                break;
            case '/':
                if (peek_next() == '/') {
                    while (peek() != '\n' && !is_at_end()) advance();
                    break;
                } else {
                    return;
                }
            default:
                return;
        }
    }
}

TokenType Scanner::check_keyword(int offset, int length, const char *expected,
                                 TokenType type) {
    int check_len = (int)(current - start - offset);
    if (check_len == length && memcmp(start + offset, expected, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

TokenType Scanner::get_identifier_type() {
    switch (*start) {
        case 'e':
            return check_keyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
            switch (start[1]) {
                case 'a': return check_keyword(2, 3, "lse", TOKEN_FALSE);
                case 'o': return check_keyword(1, 2, "or", TOKEN_FOR);
                case 'n': return TOKEN_FN;
            }
        case 'i':
            return check_keyword(1, 1, "f", TOKEN_IF);
        case 'l':
            return check_keyword(1, 2, "et", TOKEN_LET);
        case 'n':
            return check_keyword(1, 2, "il", TOKEN_NIL);
        case 'r':
            return check_keyword(1, 5, "eturn", TOKEN_RETURN);
        case 't':
            return check_keyword(1, 3, "rue", TOKEN_TRUE);
        case 'w':
            return check_keyword(1, 4, "hile", TOKEN_WHILE);
        default:
            return TOKEN_IDENTIFIER;
    }
}

Token Scanner::identifier() {
    while (is_alpha(peek()) || is_digit(peek()) || peek() == '_') advance();
    return make_token(get_identifier_type());
}

Token Scanner::string() {
    while (peek() != '"' && !is_at_end()) {
        advance();
    }

    if (peek() != '"') return make_error_token("Unterminated string.");

    // Consume the closing '"'.
    advance();

    return make_token(TOKEN_STRING);
}

Token Scanner::number() {
    while (is_digit(peek()) || peek() == '.') advance();
    return make_token(TOKEN_NUMBER);
}

Token Scanner::make_error_token(const char *error_message) {
    return Token{.type = TOKEN_ERROR,
                 .start = error_message,
                 .length = static_cast<int>(strnlen(error_message, 255)),
                 .line = line};
}

Token Scanner::make_token(TokenType type) {
    return Token{.type = type,
                 .start = start,
                 .length = static_cast<int>(current - start),
                 .line = line};
}
}  // namespace carl
