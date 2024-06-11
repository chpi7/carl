include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.13.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(TEST_CC
    test/scanner_test.cc
    test/util_test.cc
    test/parser_test.cc
    test/codegen2_test.cc
    test/polymorphic_types_test.cc
)

add_executable(tester test/tester.cc ${TEST_CC} ${CARL_INCLUDE_H})
target_link_libraries(
  tester 
  carl-lib 
  gtest 
  gmock 
  ${LLVM_LIBS}
  ${LLVM_LDFLAGS}
)

enable_testing()
add_test(carl-lib tester)
