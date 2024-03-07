#include <gtest/gtest.h>

#include <library/common/option.hpp>

#include <library/common/types.hpp>

template<typename T>
using option = library::common::option<T>;

using word = library::common::word;

TEST(option_test, option_has_value_test)
{
    auto opt = option<word>::some(word { 0x1 });
    ASSERT_EQ(true, opt.has_value());
}

TEST(option_test, option_unwrap_test)
{
    auto opt = option<word>::some(word { 0xdeadbeaf });
    ASSERT_EQ(0xdeadbeaf, opt.unwrap());
}

TEST(option_test, option_none_test)
{
    auto opt = option<word>::none();
    ASSERT_EQ(false, opt.has_value());
}
