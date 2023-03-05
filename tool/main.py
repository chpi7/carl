"""AstNode Generator (requires python 3.10)"""

from pathlib import Path
from argparse import ArgumentParser

from config import *
from config_parser import parse_classes
from ast_generator import generate_header_file, generate_cc_file
from printer_generator import generate_print_visitor

def find_root_path(start: Path) -> Path:
    git_folder = (start / ".git")
    if git_folder.exists() and git_folder.is_dir():
        return start
    elif start.parent:
        return find_root_path(start.parent)
    else:
        print("Could not find repository root.")
        return None

def resolve_paths(ps: list[Path]) -> list[Path]:
    return list(map(lambda p: p.resolve(), ps))

def write_file(content: str, path: Path):
    root = find_root_path(Path.cwd())
    full_path = root / path
    print(f"📜 Writing {full_path.as_posix()}")
    with open(path, "wt") as f:
        f.write(content)

def main():
    argparser = ArgumentParser()
    argparser.add_argument(
        "-oh", type=Path, default=Path("include/carl/ast/ast.h")
    )
    argparser.add_argument("-occ", type=Path, default=Path("src/ast/ast.cc"))
    args = argparser.parse_args()

    header_path, src_path = [args.oh, args.occ]

    print(f"🕵️‍♂️ Parsing classes...")
    classes = parse_classes(TYPES)

    print(f"🏗️  Producing code...")
    header_file = generate_header_file(classes)
    cc_file = generate_cc_file(classes)

    printer_h, printer_cc = generate_print_visitor(classes)
    printer_h_path = header_path.parent / "ast_printer.h"
    printer_cc_path = src_path.parent / "ast_printer.cc"

    write_file(header_file, header_path)
    write_file(cc_file, src_path)

    write_file(printer_h, printer_h_path)
    write_file(printer_cc, printer_cc_path)

    print(f"🚀🚀🚀 all done")


if __name__ == "__main__":
    main()
