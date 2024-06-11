cmake_minimum_required(VERSION 3.16)

project(
    carl-language
    VERSION 0.1
    DESCRIPTION "The carl programming language"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

# LLVM
find_package(LLVM 15 REQUIRED CONFIG)
message(STATUS "LLVM_VERSION = ${LLVM_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM_INCLUDE_DIRS = ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM_INSTALL_PREFIX = ${LLVM_INSTALL_PREFIX}")
# separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(LLVM_LIBS core support orcjit nativecodegen x86codegen)

include("${CMAKE_SOURCE_DIR}/include/local.cmake")
include_directories(${CMAKE_SOURCE_DIR}/include)

include_directories(SYSTEM 
    ${GTEST_INCLUDE_DIRS}
    ${LLVM_INCLUDE_DIRS}
)

include("${CMAKE_SOURCE_DIR}/src/local.cmake")
include("${CMAKE_SOURCE_DIR}/test/local.cmake")
