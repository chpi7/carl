set(SRC_CC
    src/vm/vm.cc
    src/vm/chunk.cc
    src/vm/env.cc
    src/ast/types.cc
    src/ast/ast.cc
    src/ast/print_visitor.cc
    src/ast/ast_printer.cc
    src/ast/type_inference.cc
    src/parser.cc 
    src/scanner.cc 
    src/basic_codegen.cc 
    src/llvm/codegen.cc
    src/llvm/jit.cc
    src/llvm/runtime_types.cc
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