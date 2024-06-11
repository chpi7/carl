from config import *
from config_parser import Class, ClassMember, KNOWN_CLASSES

EXCLUDE_CLASSES = ["Statement", "Expression"]

# this is filled when calling a function here
AST_CLASSES = []

def is_subclass(cls: Class, *args: tuple[str]):
    if cls.name in args:
        return True
    while cls:
        if cls.parent in args:
            return True
        else:
            cls = KNOWN_CLASSES.get(cls.parent)
    return False

def fill_ast_classes(classes: list[Class]):
    for cls in classes:
        AST_CLASSES.append(cls.name)
    AST_CLASSES.append("AstNode")

def generate_ast_node_visitor_functions(classes: list[Class]):
    fs = list()
    for cls in classes:
        f = f"    void visit_{cls.name.lower()}({cls.name}* {cls.name.lower()});"
        fs.append(f)
    return "\n".join(fs)

def generate_header(classes: list[Class]):
    return f"""#ifndef CARL_AST_PRINTER_H
#define CARL_AST_PRINTER_H

#include "carl/ast/ast.h"

namespace {NAMESPACE} {{
class AstPrinter : public AstNodeVisitor {{
   private:
    std::ostream& os;
    int indent;
   public:
    AstPrinter(std::ostream& os) : os(os), indent(0) {{}};
    void print(AstNode* node) {{
        indent = 0;
        node->accept(this);
        os << "\\n";
    }}
{generate_ast_node_visitor_functions(classes)}
private:
    void write_indent() {{
        static constexpr const char* indent_with = "  ";
        for (int i = 0; i < indent; ++i) os << indent_with;
    }}
}};
}}
#endif
    """

def generate_list_attr(cls: Class, attr: ClassMember) -> str:
    is_printable = "<int>" in attr.typename
    multi_line = f"""    write_indent();
    os << ".{attr.name}\\n";
    indent++;
    for (auto& elem : {cls.name.lower()}->get_{attr.name.lower()}()) {{
        elem->accept(this);
    }}
    indent--;"""

    one_line = f"""    write_indent();
    os << ".{attr.name} = ";
    indent++;
    for (auto& elem : {cls.name.lower()}->get_{attr.name.lower()}()) {{
        os << elem << " ";
    }}
    os << "\\n";
    indent--;"""

    return one_line if is_printable else multi_line

def generate_ptr_attr(cls: Class, attr: ClassMember) -> str:
    # assume attr.typename of this form: @ptr<...>
    inner_type = attr.typename.removeprefix("@ptr<").removesuffix(">")
    print("gen ptr attr for " + cls.name, attr.name)
    if inner_type not in AST_CLASSES:
        # for now, skip non ast classes
        return ""
    return f"""    write_indent();
    os << ".{attr.name}\\n";
    indent++;
    {cls.name.lower()}->get_{attr.name.lower()}()->accept(this);
    indent--;"""

def generate_token_attr(cls: Class, attr: ClassMember) -> str:
    return f"""    write_indent();
    os << ".{attr.name} = " << std::string({cls.name.lower()}->get_{attr.name}().start, {cls.name.lower()}->get_{attr.name}().length) << "\\n";"""

def generate_std_string_attr(cls: Class, attr: ClassMember) -> str:
    return f"""    write_indent();
    os << ".{attr.name} = " << {cls.name.lower()}->get_{attr.name}() << "\\n";"""

def generate_bool_attr(cls: Class, attr: ClassMember) -> str:
    return f"""    write_indent();
    os << ".{attr.name} = " << ({cls.name.lower()}->get_{attr.name}() ? std::string("true") : std::string("false")) << "\\n";"""

def generate_attr(cls: Class, attr: ClassMember) -> str:
    is_list = "@list" in attr.typename or "@vec" in attr.typename
    is_token = "Token" in attr.typename
    is_std_string = attr.typename == "std::string" 
    is_bool = attr.typename == "bool"
    
    if is_list:
        return generate_list_attr(cls, attr)
    elif is_token:
        return generate_token_attr(cls, attr)
    elif is_std_string:
        return generate_std_string_attr(cls, attr)
    elif is_bool:
        return generate_bool_attr(cls, attr)
    else:
        return generate_ptr_attr(cls, attr)

def generate_impl(cls: Class) -> str:
    print_type = is_subclass(cls, "Expression", "FormalParam", "LetDecl", "FnDecl", "AdtStmt")
    cls_name_with_type = f"""os << "{cls.name} [" << {cls.name.lower()}->get_type()->str() << "]\\n";"""
    cls_name = f"""os << "{cls.name}" << "\\n";"""
    attrs = "\n".join(map(lambda m: generate_attr(cls, m), cls.members))
    return f"""void AstPrinter::visit_{cls.name.lower()}({cls.name}* {cls.name.lower()}) {{
    write_indent();
    {cls_name_with_type if print_type else cls_name} 
    indent++;
{attrs}
    indent--;
}}"""

def generate_impls(classes: list[Class]) -> str:
    impls = "\n\n".join(map(generate_impl, classes))
    return f"""#include "carl/ast/ast_printer.h"

using namespace {NAMESPACE};

{impls}
"""


def generate_print_visitor(classes: list[Class]) -> list[str]:
    fill_ast_classes(classes)
    to_generate = list(filter(lambda c: c.name not in EXCLUDE_CLASSES, classes))
    return generate_header(to_generate), generate_impls(to_generate)
