#include "carl/parser.h"

#include <list>
#include <unordered_map>
#include <unordered_set>

#include "carl/ast/type_inference.h"
#include "carl/common.h"

/* Grammar Excerpt

    program -> declaration*
    declaration -> letDecl | fnDecl | statement

    statement -> whileStatment | blockStatement | expressionStatement
    whileStatement -> 'while' ( expression ) statement
    blockStatement -> { declaration* }
    expressionStatement -> statement ';'
*/

namespace carl {

template <typename T>
static std::shared_ptr<T> make_error_node() {
    return nullptr;
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

// Parse rules for the parse_precedence function --> returns expressions only.
static const std::unordered_map<TokenType, ParseRule> parse_rules = {
    {TOKEN_LEFT_PAREN, {PREC_NONE, &Parser::grouping, nullptr}},
    {TOKEN_RIGHT_PAREN, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_LEFT_BRACE, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_RIGHT_BRACE, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_COMMA, {PREC_NONE, nullptr, nullptr}},
    {TOKEN_DOT, {PREC_COMPOSITION, nullptr, &Parser::binary}},
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
        advance();  // dont get stuck in infinite loop
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
        expression = (this->*(prefix_rule->prefix))();
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
    environment = std::make_unique<Environment<Variable*>>(nullptr);
    fn_environment = std::make_unique<Environment<FnDecl*>>(nullptr);
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

ParseResult Parser::parse_r(std::string& src, bool add_builtins) {
    std::shared_ptr<carl::FnDecl> puts_decl, debug_decl, assert_decl;
    if (add_builtins) {
        puts_decl =
            decl_builtin("__puts", {std::make_shared<types::String>()},
                         std::make_shared<types::Int>());
        debug_decl =
            decl_builtin("__debug", {std::make_shared<types::Int>()},
                         std::make_shared<types::Void>());
        assert_decl =
            decl_builtin("__assert", {std::make_shared<types::Bool>()},
                         std::make_shared<types::Void>());
    }

    auto decls = parse(src);
    if (add_builtins) {
        decls.insert(decls.begin(), puts_decl);
        decls.insert(decls.begin(), debug_decl);
        decls.insert(decls.begin(), assert_decl);
    }

    if (has_error) {
        return ParseResult::make_error(ParseError{"some error occured"});
    }

    TypeInference ti;
    auto r = ti.run(decls);
    if (!r) {
        return ParseResult::make_error(
            ParseError{.message = r.get_error().message});
    }

    return ParseResult::make_result(decls);
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
        return {};  // std::vector<std::shared_ptr<AstNode>>();
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

std::shared_ptr<types::Type> Parser::type() {
    if (match(TOKEN_LEFT_PAREN)) {
        // (int, string : void)
        std::vector<std::shared_ptr<types::Type>> param_types;
        while (!peek(TOKEN_COLON)) {
            std::shared_ptr<types::Type> t = type();
            param_types.push_back(t);
            if (match(TOKEN_COMMA)) {
                // more args to come
                continue;
            } else {
                break;
            }
        }
        consume(TOKEN_COLON,
                "Expected : between function param types and return type");
        auto ret_type = type();
        consume(TOKEN_RIGHT_PAREN,
                "Expected ) after function type return type.");
        return std::make_shared<types::Fn>(param_types, ret_type);
    } else {
        consume(TOKEN_IDENTIFIER,
                "Expected identifier or fn type as typename.");
        std::string type_str = previous;
        return type_from_identifier(type_str);
    }
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
    if (!environment->can_set_variable(name)) {
        error_at(previous, "Redeclaration of variable not allowed.");
    } else {
        // safe the type here when we have it?
        environment->set_variable(name, nullptr);
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

std::shared_ptr<FnDecl> Parser::fn_decl() {
    consume(TOKEN_IDENTIFIER, "Expected function name after fn keyword.");
    auto name = previous;

    auto fn_name = std::string(name.start, name.length);
    fn_environment->set_variable(fn_name, nullptr);

    consume(TOKEN_LEFT_PAREN, "Expected ( after fn name.");

    UseNewEnv eh(environment.get());
    UseNewEnv feh(fn_environment.get());

    std::list<std::shared_ptr<FormalParam>> formal_params;
    std::vector<std::shared_ptr<types::Type>> formal_param_types;
    while (!peek(TOKEN_RIGHT_PAREN)) {
        if (formal_params.size() > 0) {
            consume(TOKEN_COMMA, "Expected ',' after formal parameter.");
        }

        if (!match(TOKEN_IDENTIFIER)) {
            error_at(current, "Expected identifier as formal parameter.");
            return make_error_node<FnDecl>();
        }
        auto fp = std::make_shared<FormalParam>(previous);
        auto fp_name = std::string(previous.start, previous.length);
        environment->set_variable(fp_name, nullptr);

        consume(
            TOKEN_COLON,
            "Expected ':' between formal param name and formal param type.");
        auto fp_type = type();
        formal_param_types.push_back(fp_type);
        fp->set_type(fp_type);

        formal_params.push_back(fp);
    }
    consume(TOKEN_RIGHT_PAREN, "Expected ) after fn formal parameters.");

    // return type
    std::shared_ptr<types::Type> fn_ret_type = std::make_shared<types::Void>();
    if (match(TOKEN_COLON)) {
        fn_ret_type = type();
    }

    // body
    consume(TOKEN_LEFT_BRACE, "Expected block as function body.");
    if (has_error) {
        return make_error_node<FnDecl>();
    }

    current_fn_env_id.push_back(environment->get_id());
    captured_variables.emplace_back();
    auto body = block();
    auto captures = captured_variables.back();
    auto unique_captures = std::list<std::shared_ptr<Variable>>();

    std::unordered_set<std::string> known_captures;
    for (auto& capture : captures) {
        std::string name = capture->get_name();
        if (!known_captures.contains(name)) {
            unique_captures.push_back(capture);
            known_captures.insert(name);
        }
    }

    captured_variables.pop_back();
    current_fn_env_id.pop_back();

    if (has_error) {
        return make_error_node<FnDecl>();
    }
    auto fn = std::make_shared<FnDecl>(name, formal_params, body);
    fn->set_captures(unique_captures);
    auto fn_type = std::make_shared<types::Fn>(formal_param_types, fn_ret_type);
    fn->set_type(fn_type);
    return fn;
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
    UseNewEnv eh(environment.get());
    UseNewEnv feh(fn_environment.get());

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

std::shared_ptr<Expression> Parser::call() {
    consume(TOKEN_IDENTIFIER, "Expected function identifier.");
    Token fname = previous;
    std::string fname_str = fname;

    // var can hold a function --> check both:
    if (!fn_environment->has_variable(fname_str) &&
        !environment->has_variable(fname_str)) {
        error_at(previous, "Function name not found in environment");
    }

    consume(TOKEN_LEFT_PAREN, "Expected '(' after function identifier.");

    auto args = std::list<std::shared_ptr<Expression>>();
    while (current.type != TOKEN_ERROR && current.type != TOKEN_EOF &&
           current.type != TOKEN_RIGHT_PAREN) {
            args.push_back(expression());
        if (!match(TOKEN_COMMA)) break;
    }
    consume(TOKEN_RIGHT_PAREN,
            "Expected ) at the end of function argument list.");

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
    if (!environment->has_variable(name) &&
        !fn_environment->has_variable(name)) {
        error_at(previous, "name not found in environment");
    }

    auto variable = std::make_shared<Variable>(previous);
    if (environment->has_variable(name) && is_captured(name)) {
        // std::cout << "detected " << name << " as captured.\n";
        captured_variables.back().push_back(variable);
    }
    return variable;
}

std::shared_ptr<Expression> Parser::binary() {
    const ParseRule* current_rule = get_rule(previous.type);

    // make some operator right associative
    int prec_offset = 0;
    Precedence current = current_rule->prec;
    switch (current) {
        case PREC_ASSIGNMENT:
        case PREC_COMPOSITION:
            break;
        default:
            prec_offset += 1;
    }

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

bool Parser::is_captured(const std::string& name) {
    if (current_fn_env_id.empty()) return false;
    // Check up to the env of the function if the name is known.
    // If not it has to come from outside of the function.
    return !environment->has_variable(name, current_fn_env_id.back());
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

std::shared_ptr<FnDecl> Parser::decl_builtin(
    const std::string& name,
    std::vector<std::shared_ptr<types::Type>> param_types,
    std::shared_ptr<types::Type> return_type) {
    Token t;
    t.start = name.c_str();
    t.length = name.size();

    std::list<std::shared_ptr<FormalParam>> p;
    for (auto& param_type : param_types) {
        auto fp = std::make_shared<FormalParam>(t);
        fp->set_type(param_type);
    }

    std::list<std::shared_ptr<AstNode>> d;
    auto puts_decl = std::make_shared<FnDecl>(t, p, std::make_shared<Block>(d));
    puts_decl->set_is_extern(true);

    auto x = std::make_shared<types::Fn>(param_types, return_type);
    puts_decl->set_type(x);
    fn_environment->set_variable(name, puts_decl.get());

    return puts_decl;
}

}  // namespace carl
