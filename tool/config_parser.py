from dataclasses import dataclass

@dataclass
class ClassMember:
    typename: str
    name: str
    is_optional: bool = False
    default: str | None = None

@dataclass
class Class:
    name: str
    parent_cls: "Class"
    parent: str
    members: list[ClassMember]

KNOWN_CLASSES = {}

def parse_class_decl(decl: str) -> Class:
    decl.replace("\n", " ")
    rem, parent = decl.split(" : ")
    parent = parent.strip()

    name, rem = rem.split("(", maxsplit=1)
    name = name.strip()
    rem = rem.strip()
    assert rem[-1] == ")", f"Expected variable list to have a ) at the end. is: {rem}"
    rem = rem[:-1]

    members = rem.strip().split(",")
    if members == [""]:
        members = list()

    def parse_member(m: str) -> ClassMember:
        type_, name = m.strip().split(" ")
        default = None
        if "=" in name:
            name, default = name.split("=")
        is_optional = name.endswith("?")
        if is_optional:
            name = name[:-1]
        return ClassMember(type_, name, is_optional, default)

    pcls = KNOWN_CLASSES.get(parent)

    r = Class(name, pcls, parent, list(map(parse_member, members)))
    KNOWN_CLASSES[name] = r

    return r


def parse_classes(decls: list[str]) -> list[Class]:
    KNOWN_CLASSES.clear()
    return list(map(parse_class_decl, decls))