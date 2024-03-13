#include <gtest/gtest.h>

#include <library/common/option.hpp>

#include <library/common/types.hpp>

template<typename T>
using option = library::common::option<T>;

using word = library::common::word;

TEST(option_test, option_has_value_test)
{
    auto opt = library::common::make_option_some(0x40);
    ASSERT_EQ(true, opt.has_value());
}

TEST(option_test, option_unwrap_test)
{
    auto opt = library::common::make_option_some(word { 0xdeadbeaf });
    ASSERT_EQ(0xdeadbeaf, opt.unwrap());
}

TEST(option_test, option_none_test)
{
    option<word> opt = library::common::make_option_none();
    ASSERT_EQ(false, opt.has_value());
}

TEST(option_test, option_ptr_test)
{
    word base_value = 400;
    word *base_value_address = &base_value;
    option<word *> opt = library::common::make_option_some(base_value_address);

    auto addr = opt.unwrap();
    ASSERT_EQ(400, *addr);
}
