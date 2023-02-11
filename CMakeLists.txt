cmake_minimum_required(VERSION 3.16)

project(
    carl-language
    VERSION 0.1
    DESCRIPTION "The carl programming language"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include("${CMAKE_SOURCE_DIR}/include/local.cmake")
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories(SYSTEM 
    ${GTEST_INCLUDE_DIRS}
)

include("${CMAKE_SOURCE_DIR}/src/local.cmake")
include("${CMAKE_SOURCE_DIR}/test/local.cmake")
