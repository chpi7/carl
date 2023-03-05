from dataclasses import dataclass

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
    rem, parent = decl.split(" : ")
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