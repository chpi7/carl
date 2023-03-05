IFDEF_NAME = "carl_ast_h"
INCLUDES = ["<sstream>", "<fstream>", "<memory>", "<list>", '"carl/scanner.h"', '"carl/common.h"']
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
    "Expression() : AstNode",

    "Type(Token name) : AstNode",
    "FormalParam(Token name, @ptr<Type> type) : AstNode",
    "FnDecl(Token name, @list<@ptr<AstNode>> formals, @ptr<AstNode> body) : AstNode",

    # TODO: rename this to LetDecl
    "LetStmt(Token name, @ptr<AstNode> initializer) : Statement",
    "ExprStmt(@ptr<AstNode> expr) : Statement",
    "ReturnStmt(@ptr<AstNode> expr) : Statement",
    "WhileStmt(@ptr<AstNode> condition, @ptr<AstNode> body) : Statement",
    "Block(@list<@ptr<AstNode>> declarations) : Statement",

    "Assignment(@ptr<AstNode> target, @ptr<Expression> expr) : Expression",
    "Binary(Token op, @ptr<Expression> lhs, @ptr<Expression> rhs) : Expression",
    "Unary(Token op, @ptr<Expression> operand) : Expression",
    "Variable(Token name) : Expression",
    "Literal(Token value) : Expression",
    "String(Token value) : Expression",
    "Number(Token value) : Expression",
    "Call(Token fname, @list<Expression> arguments) : Expression",
]