set(SRC_CC
    src/ast.cc
    src/print_visitor.cc
    src/parser.cc 
    src/scanner.cc 
    src/vm.cc
    src/instruction.cc
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

target_link_libraries(carl carl-lib)