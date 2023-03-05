"""AstNode Generator (requires python 3.10)"""

from pathlib import Path
from argparse import ArgumentParser

from config import *
from config_parser import parse_classes
from ast_generator import generate_header_file, generate_cc_file

def main():
    argparser = ArgumentParser()
    argparser.add_argument(
        "-oh", type=Path, default=Path(__file__, "../../include/carl/ast/ast.h")
    )
    argparser.add_argument("-occ", type=Path, default=Path(__file__, "../../src/ast/ast.cc"))
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
