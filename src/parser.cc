#include "carl/parser.h"

#include <unordered_map>
#include <list>

#include "carl/common.h"
#include "carl/ast/types.h"

/* Grammar Excerpt

    program -> declaration*
    declaration -> letDecl | fnDecl | statement

    statement -> whileStatment | blockStatement | expressionStatement 
    whileStatement -> 'while' ( expression ) statement
    blockStatement -> { declaration* }
    expressionStatement -> statement ';'
*/

namespace carl {

template<typename T>
static std::shared_ptr<T> make_error_node() {
    return nullptr;
}

// Parse rules for the parse_precedence function --> returns expressions only.
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

std::shared_ptr<Expression> Parser::parse_precedence(Precedence precedence) {
    auto prefix_rule = get_rule(current.type);
    // assert(get_rule(current.type)->prefix != nullptr && "No prefix rule for
    // token");
    if (prefix_rule->prefix == nullptr) {
        error_at(current, "no prefix rule found.");
        advance(); // dont get stuck in infinite loop
        return make_error_node<Expression>();
    }

    std::shared_ptr<Expression> expression;
    if (peek(TOKEN_IDENTIFIER) && peek_next(TOKEN_LEFT_PAREN)) {
        // function call
        expression = call();
        // can not assign to Result of call.
        if (precedence == PREC_ASSIGNMENT) {
            precedence = static_cast<Precedence>(precedence + 1);
        }
    } else {
        // so nice :)
        expression =
            (this->*(prefix_rule->prefix))();
    }

    const ParseRule* infix_rule;
    while (precedence <= (infix_rule = get_rule(current.type))->prec) {
        assert(get_rule(current.type)->infix != nullptr &&
               "No infix rule for token");

        auto op_token = current;
        advance();

        if (op_token.type == TOKEN_EQUAL) {
            if (precedence > PREC_ASSIGNMENT) {
                error_at(current, "assignment no possible here");
                return make_error_node<Expression>();
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
    environment = std::make_unique<Environment>(nullptr);
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

ParseResult Parser::parse_r(std::string& src) {
    auto decls = parse(src);
    if (has_error) {
        return ParseResult::make_error(ParseError{"some error occured"});
    } else {
        return ParseResult::make_result(decls);
    }
}

std::vector<std::shared_ptr<AstNode>> Parser::parse(std::string& src) {
    auto scanner = std::make_shared<Scanner>();
    scanner->init(src.c_str());
    set_scanner(scanner);
    
    return parse();
}

std::vector<std::shared_ptr<AstNode>> Parser::parse() {
    if (!scanner) {
        std::cerr << "scanner not set." << std::endl;
        return {}; // std::vector<std::shared_ptr<AstNode>>();
    }
    std::vector<std::shared_ptr<AstNode>> result;
    while (current.type != TOKEN_EOF) {
        result.push_back(declaration());
        if (panic_mode) {
            synchronize();
        }
    }
    return result;
}

std::shared_ptr<Type> Parser::type() {
    consume(TOKEN_IDENTIFIER, "Expected identifier as typename.");
    return std::make_shared<Type>(previous);
}

std::shared_ptr<AstNode> Parser::declaration() {
    if (match(TOKEN_LET)) {
        return let_decl();
    } else if (match(TOKEN_FN)) {
        return fn_decl();
    } else {
        return statement();
    }
}

std::shared_ptr<LetDecl> Parser::let_decl() {
    consume(TOKEN_IDENTIFIER,
            "Expected identifier as variable name after let.");
    auto identifier = previous;
    auto name = std::string(identifier.start, identifier.length);
    if (environment->has_variable(name)) {
        error_at(previous, "Redeclaration of name not allowed.");
    } else {
        // safe the type here when we have it?
        environment->set_variable(name);
    }

    if (peek(TOKEN_SEMICOLON)) {
        error_at(current,
                 "Expected initialization after variable declaration.");
        return make_error_node<LetDecl>();
    } else if (match(TOKEN_EQUAL)) {
        auto initializer = expression();
        auto result = std::make_shared<LetDecl>(identifier, initializer);
        if (!match(TOKEN_SEMICOLON)) {
            error_at(current, "Expected ; after let initializer.");
        }
        return result;
    } else {
        error_at(current, "Expected '=' after 'let'.");
        return make_error_node<LetDecl>();
    }
}

static std::shared_ptr<types::Type> type_from_identifier(std::string& s) {
    if (s == std::string("int")) {
        return std::make_shared<types::Int>();
    } else if (s == std::string("float")) {
        return std::make_shared<types::Float>();
    } else if (s == std::string("bool")) {
        return std::make_shared<types::Bool>();
    } else if (s == std::string("string")) {
        return std::make_shared<types::String>();
    } else {
        // Todo function types
        return std::make_shared<types::Unknown>();
    }
}

std::shared_ptr<FnDecl> Parser::fn_decl() {

    consume(TOKEN_IDENTIFIER, "Expected function name after fn keyword.");
    auto name = previous;

    // add to env immediatly to allow recursive calls
    auto fn_name = std::string(name.start, name.length);
    environment->set_function(fn_name);

    consume(TOKEN_LEFT_PAREN, "Expected ( after fn name.");

    UseNewEnv eh(this);

    std::list<std::shared_ptr<FormalParam>> formal_params;
    while (!peek(TOKEN_RIGHT_PAREN)) {
        if (formal_params.size() > 0) {
            consume(TOKEN_COMMA, "Expected ',' after formal parameter.");
        }

        if (!match(TOKEN_IDENTIFIER)) {
            error_at(current, "Expected identifier as formal parameter.");
            return make_error_node<FnDecl>();
        }
        auto fp_name = std::string(previous.start, previous.length);
        environment->set_variable(fp_name);

        consume(TOKEN_COLON, "Expected ':' between formal param name and formal param type.");
        auto type_ = type();
        auto type_str = std::string(type_->get_name().start, type_->get_name().length);
        auto fp = std::make_shared<FormalParam>(previous);
        fp->set_type(type_from_identifier(type_str));

        formal_params.push_back(fp);
    }
    consume(TOKEN_RIGHT_PAREN, "Expected ) after fn formal parameters.");

    auto body = statement();
    if (has_error) {
        return make_error_node<FnDecl>();
    } else {
        return std::make_shared<FnDecl>(name, formal_params, body);
    }
}

std::shared_ptr<Statement> Parser::statement() {
    if (match(TOKEN_WHILE)) {
        return while_stmt();
    } else if (match(TOKEN_LEFT_BRACE)) {
        return block();
    } else if (match(TOKEN_RETURN)) {
        return ret_stmt();
    } else {
        return expr_stmt();
    }
}

std::shared_ptr<ReturnStmt> Parser::ret_stmt() {
    auto return_value = expression();
    consume(TOKEN_SEMICOLON, "Expected ';' at the end of statement.");
    return std::make_shared<ReturnStmt>(return_value);
}

std::shared_ptr<WhileStmt> Parser::while_stmt() {
    consume(TOKEN_LEFT_PAREN, "Expected ( after 'while'.");
    auto condition = expression();
    consume(TOKEN_RIGHT_PAREN, "Expected ) after while condition.");
    auto body = statement();
    return std::make_shared<WhileStmt>(condition, body);
}

std::shared_ptr<Block> Parser::block() {
    UseNewEnv eh(this);

    std::list<std::shared_ptr<AstNode>> decls;
    while (!peek(TOKEN_RIGHT_BRACE)) {
        decls.push_back(declaration());
        if (has_error) break;
    }
    consume(TOKEN_RIGHT_BRACE, "Expected } at the end of a block.");
    return std::make_shared<Block>(decls);
}

std::shared_ptr<ExprStmt> Parser::expr_stmt() {
    auto expr = expression();
    consume(TOKEN_SEMICOLON, "Expected ';' at the end of statement.");
    return std::make_shared<ExprStmt>(expr);
}

std::shared_ptr<Expression> Parser::expression() {
    return parse_precedence(PREC_ASSIGNMENT);
}

std::shared_ptr<Call> Parser::call() {
    consume(TOKEN_IDENTIFIER, "Expected function identifier.");
    auto fname = previous;

    auto fname_str = std::string(fname.start, fname.length);
    if (!environment->has_function(fname_str)) {
        error_at(previous, "Function name not found in environment");
    }

    consume(TOKEN_LEFT_PAREN, "Expected '(' after function identifier.");

    auto args = std::list<std::shared_ptr<Expression>>();
    bool first = true;
    do {
        args.push_back(expression());
        if (!match(TOKEN_COMMA)) break;
    } while (current.type != TOKEN_ERROR && current.type != TOKEN_EOF);
    consume(TOKEN_RIGHT_PAREN, "Expected ) at the end of function argument list.");

    return std::make_shared<Call>(fname, args);
}

std::shared_ptr<Expression> Parser::literal() {
    advance();

    auto literal = std::make_shared<Literal>(previous);

    switch (previous.type) {
        case TOKEN_TRUE:
        case TOKEN_FALSE:
            literal->set_type(std::make_shared<types::Bool>());
            break;
        case TOKEN_NIL:
            error_at(previous, "there is no support for nil atm.");
            break;
        default:
            error_at(previous, "Can not determine literal type.");
            break;
    }

    return literal;
}

std::shared_ptr<Expression> Parser::number() {
    advance();
    auto num_str = std::string(previous.start, previous.length);
    bool is_float = num_str.find('.') != std::string::npos;

    auto number = std::make_shared<Number>(previous);
    if (is_float) {
        number->set_type(std::make_shared<types::Float>());
    } else {
        number->set_type(std::make_shared<types::Int>());
    }
    return number;
}

std::shared_ptr<Expression> Parser::string() {
    advance();
    auto s = std::make_shared<String>(previous);
    s->set_type(std::make_shared<types::String>());
    return s;
}

std::shared_ptr<Expression> Parser::unary() {
    advance();  // Consume the unary operator.
    auto op_token = previous;
    return std::make_shared<Unary>(op_token, parse_precedence(PREC_UNARY));
}

std::shared_ptr<Expression> Parser::variable() {
    consume(TOKEN_IDENTIFIER, "Expected identifier as variable name.");
    std::string name = std::string(previous.start, previous.length);
    if (!environment->has_variable(name)) {
        error_at(previous, "name not found in environment");
    }
    return std::make_shared<Variable>(previous);
}

std::shared_ptr<Expression> Parser::binary() {
    const ParseRule* current_rule = get_rule(previous.type);

    // small trick to make assignment right associative
    int prec_offset = 0;
    if (current_rule->prec != PREC_ASSIGNMENT) prec_offset++;

    return parse_precedence(
        static_cast<Precedence>(current_rule->prec + prec_offset));
}

std::shared_ptr<Expression> Parser::grouping() {
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

bool Parser::peek_next(TokenType tokenType) {
    if (scanner->peek_token().type != tokenType) return false;
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

void Parser::push_env() {
    auto new_env = std::make_unique<Environment>(std::move(environment));
    environment = std::move(new_env);
}

void Parser::pop_env() {
    if (environment) environment = environment->destroy(); 
}
}  // namespace carl