#include "carl/common.h"

#include <gtest/gtest.h>

using namespace carl;

namespace {

TEST(ResultGeneric, basic_test) {
    auto okay = Result<int, int>::make_result(1);
    auto error = Result<int, int>::make_error(2);

    ASSERT_TRUE(okay);
    ASSERT_FALSE(error);

    ASSERT_EQ(*okay, 1);
    ASSERT_EQ(error.get_error(), 2);
}

TEST(ResultGeneric, with_string_message) {
    auto error = Result<int, std::string>::make_error("my error message");
    ASSERT_EQ(error.get_error(), "my error message");
}

}