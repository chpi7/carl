set(SRC_CC
    src/ast/types.cc
    src/ast/ast.cc
    src/ast/print_visitor.cc
    src/ast/ast_printer.cc
    src/ast/type_inference.cc
    src/jit2/codegen2.cc
    src/jit2/carljit.cc
    src/parser.cc 
    src/scanner.cc 
)

add_library(
    carl-lib
    STATIC
    ${SRC_CC}
    ${INCLUDE_H}
)

add_executable(
    carl
    src/carl.cc
    ${INCLUDE_H}
)

target_link_libraries(
    carl 
    carl-lib
    ${LLVM_LIBS}
    ${LLVM_LDFLAGS}
)
