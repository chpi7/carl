cmake_minimum_required(VERSION 3.16)

project(carl)

include_directories(include)

add_executable(clox 
    src/main.c 
    src/chunk.c 
    src/debug.c 
    src/memory.c
    src/value.c
    src/vm.c
    src/compiler.c
    src/scanner.c
    src/object.c
)