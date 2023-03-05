"""AstNode Generator (requires python 3.10)"""

from dataclasses import dataclass
from pathlib import Path
from argparse import ArgumentParser

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


@dataclass
class Class:
    name: str
    parent: str
    members: list["ClassMember"]


@dataclass
class ClassMember:
    typename: str
    name: str


def parse_class_decl(decl: str) -> Class:
    rem, parent = decl.split(":")
    parent = parent.strip()

    name, rem = rem.split("(")
    name = name.strip()
    rem = rem.strip()
    assert rem[-1] == ")", f"Expected variable list to have a ) at the end. is: {rem}"
    rem = rem[:-1]

    members = rem.strip().split(",")
    if members == [""]:
        members = list()

    def parse_member(m: str) -> ClassMember:
        return ClassMember(*m.strip().split(" "))

    return Class(name, parent, list(map(parse_member, members)))


def parse_classes(decls: list[str]) -> list[Class]:
    return list(map(parse_class_decl, decls))


def generate_constructor(cls: Class):
    r = cls.name
    r += "("
    r += ", ".join(map(lambda m: f"{m.typename} {m.name}", cls.members))
    r += f")"
    if cls.members:
        r += " :"
        r += ", ".join(map(lambda m: f"{m.name}({m.name})", cls.members))
    r += " {}"
    return r


def generate_member_decls(members: list[ClassMember]):
    ds = list()
    for m in members:
        d = f"    {m.typename} {m.name};"
        ds.append(d)
    return "\n".join(ds)


def generate_member_getters(members: list[ClassMember]):
    ds = list()
    for m in members:
        d = f"    {m.typename} get_{m.name}() {{ return this->{m.name}; }}"
        ds.append(d)
    return "\n".join(ds)


def generate_class_definition(cls: Class):
    template = f"""class {cls.name} : public {cls.parent} {{
   private:
{generate_member_decls(cls.members)}
   public:
    {generate_constructor(cls)}
{generate_member_getters(cls.members)}
    void accept(AstNodeVisitor* visitor);
}};
"""
    return template


def generate_ast_node_visitor_functions(classes: list[Class]):
    fs = list()
    for cls in classes:
        f = f"    virtual void visit_{cls.name.lower()}({cls.name}* {cls.name.lower()}) {{ assert(false && \"visit {cls.name.lower()} not overwritten\"); }};"
        fs.append(f)
    return "\n".join(fs)


def generate_ast_node_visitor(classes: list[Class], name="AstNodeVisitor"):
    return f"""class {name} {{
   public:
{generate_ast_node_visitor_functions(classes)}
}};
"""


def generate_ast_node_accept_impls(classes: list[Class]):
    l = list()
    for cls in classes:
        l.append(
            f"void {cls.name}::accept(AstNodeVisitor* visitor) {{ visitor->visit_{cls.name.lower()}(this); }}"
        )
    return "\n".join(l)


def gen_header_header(name: str):
    return f"""#ifndef {name}
#define {name}
"""


def gen_include(name: str):
    return f"#include {name}"


def apply_replacements(s: str) -> str:
    for f, t in REPLACEMENTS.items():
        s = s.replace(f, t)
    return s


def generate_header_file(classes: list[Class]):
    file_content = gen_header_header(IFDEF_NAME)
    file_content += "\n"
    file_content += "\n".join(map(gen_include, INCLUDES))
    file_content += "\n\n"
    file_content += f"namespace {NAMESPACE} {{\n"
    file_content += "\n"
    file_content += "\n".join(FORWARD_DECLS)
    file_content += "\n\n"
    file_content += ASTNODE
    file_content += "\n\n"

    for cls in classes:
        impl = generate_class_definition(cls)
        file_content += apply_replacements(impl)
        file_content += "\n"

    impl = generate_ast_node_visitor(classes)
    file_content += apply_replacements(impl)
    file_content += "\n"

    file_content += f"}} // namespace {NAMESPACE}\n"
    file_content += f"#endif"

    return file_content


def generate_cc_file(classes: list[Class]):
    file_content = '#include "carl/ast/ast.h"\n\n'
    file_content += f"namespace {NAMESPACE} {{\n\n"

    file_content += generate_ast_node_accept_impls(classes)
    file_content += "\n\n"

    file_content += f"}} // namespace {NAMESPACE}\n"

    return file_content


def main():
    argparser = ArgumentParser()
    argparser.add_argument(
        "-oh", type=Path, default=Path(__file__, "../include/carl/ast/ast.h")
    )
    argparser.add_argument("-occ", type=Path, default=Path(__file__, "../src/ast/ast.cc"))
    args = argparser.parse_args()

    header_path = args.oh.resolve()
    src_path = args.occ.resolve()

    classes = parse_classes(TYPES)
    header_file = generate_header_file(classes)
    cc_file = generate_cc_file(classes)

    with open(header_path, "wt") as hf:
        hf.write(header_file)

    with open(src_path, "wt") as hf:
        hf.write(cc_file)


if __name__ == "__main__":
    main()
