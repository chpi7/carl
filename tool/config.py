IFDEF_NAME = "carl_ast_h"
INCLUDES = ["<sstream>", "<fstream>", "<memory>", "<list>", '"carl/scanner.h"', '"carl/common.h"', '"carl/ast/types.h"']
NAMESPACE = "carl"
FORWARD_DECLS = ["class AstNodeVisitor;"]
REPLACEMENTS = {"@ptr": "std::shared_ptr", "@list": "std::list"}

ASTNODE = """class AstNode {
   public:
    virtual ~AstNode() = default;
    virtual void accept(AstNodeVisitor* visitor) = 0;
};"""

TYPES = [
    "Invalid() : AstNode",

    "Statement() : AstNode",
    "Expression(@ptr<types::Type> type?) : AstNode",

    "Type(Token name) : AstNode",
    "FormalParam(Token name, @ptr<Type> type) : AstNode",
    "FnDecl(Token name, @list<@ptr<FormalParam>> formals, @ptr<Statement> body) : AstNode",
    "LetDecl(Token name, @ptr<Expression> initializer) : AstNode",

    "ExprStmt(@ptr<Expression> expr) : Statement",
    "ReturnStmt(@ptr<Expression> expr) : Statement",
    "WhileStmt(@ptr<Expression> condition, @ptr<Statement> body) : Statement",
    "Block(@list<@ptr<AstNode>> declarations) : Statement",

    "Assignment(@ptr<AstNode> target, @ptr<Expression> expr) : Expression",
    "Binary(Token op, @ptr<Expression> lhs, @ptr<Expression> rhs) : Expression",
    "Unary(Token op, @ptr<Expression> operand) : Expression",
    "Variable(Token name) : Expression",
    "Literal(Token value) : Expression",
    "String(Token value) : Expression",
    "Number(Token value) : Expression",
    "Call(Token fname, @list<@ptr<Expression>> arguments) : Expression",
]