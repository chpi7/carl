set(SRC_CC
    src/vm/vm.cc
    src/vm/chunk.cc
    src/vm/env.cc
    src/ast/ast.cc
    src/ast/print_visitor.cc
    src/ast/ast_printer.cc
    src/parser.cc 
    src/scanner.cc 
    src/codegen.cc 
    src/llvm/codegen.cc
    src/llvm/jit.cc
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