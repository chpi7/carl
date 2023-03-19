IFDEF_NAME = "carl_ast_h"
INCLUDES = ["<sstream>", "<fstream>", "<memory>", "<list>", "<vector>", "<string>", '"carl/scanner.h"', '"carl/common.h"', '"carl/ast/types.h"']
NAMESPACE = "carl"
FORWARD_DECLS = ["class AstNodeVisitor;"]
REPLACEMENTS = {"@ptr": "std::shared_ptr", "@list": "std::list", "@vec": "std::vector", 
                "@tok_to_sname_init": "std::string(name.start,name.length)"}

ASTNODE = """class AstNode {
   public:
    virtual ~AstNode() = default;
    virtual void accept(AstNodeVisitor* visitor) = 0;
    virtual AstNodeType get_node_type() const = 0;
};"""

TYPES = [
    "Statement() : AstNode",
    "Block(@list<@ptr<AstNode>> declarations) : Statement",
    "Expression(@ptr<types::Type> type?=std::make_shared<types::Unknown>()) : AstNode",

    "Type(Token name) : AstNode",
    "FormalParam(Token name, @ptr<types::Type> type?=std::make_shared<types::Unknown>()) : AstNode",
    """FnDecl(
        Token name, 
        std::string sname?=@tok_to_sname_init,
        @list<@ptr<FormalParam>> formals, 
        @ptr<Block> body, 
        @ptr<types::Type> type?=std::make_shared<types::Unknown>(),
        bool is_extern?=false
    ) : AstNode""",
    """LetDecl(
        Token name,
        @ptr<Expression> initializer,
        @ptr<types::Type> type?=std::make_shared<types::Unknown>()
    ) : AstNode""",

    "ExprStmt(@ptr<Expression> expr) : Statement",
    "ReturnStmt(@ptr<Expression> expr) : Statement",
    "WhileStmt(@ptr<Expression> condition, @ptr<Statement> body) : Statement",

    "Assignment(@ptr<Expression> target, @ptr<Expression> expr) : Expression",
    "Binary(Token op, @ptr<Expression> lhs, @ptr<Expression> rhs) : Expression",
    "Unary(Token op, @ptr<Expression> operand) : Expression",
    "Variable(Token name) : Expression",
    "Literal(Token value) : Expression",
    "String(Token value) : Expression",
    "Number(Token value) : Expression",
    "Call(Token fname, @list<@ptr<Expression>> arguments) : Expression",
    """PartialApp(
        Token fname,
        @vec<int> placeholder_positions,
        @list<@ptr<Expression>> arguments
    ) : Expression""",
]