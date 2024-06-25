#include <gtest/gtest.h>

#include <library/common/option.hpp>

#include <library/common/types.hpp>

template<typename T>
using option = library::common::option<T>;
namespace libh5n = library::common;

using word = library::common::word;

TEST(option_test, is_some_test)
{
    libh5n::option opt { 0x40 };

    ASSERT_EQ(true, opt.is_some());
}

TEST(option_test, unwrap_test)
{
    libh5n::option opt { 0xdeadbeaf };

    ASSERT_EQ(0xdeadbeaf, opt.unwrap());
}

TEST(option_test, none_test)
{
    libh5n::option opt = libh5n::option_none;

    ASSERT_EQ(false, opt.is_some());
}

TEST(option_test, pointer_test)
{
    word base_value = 400;
    word *base_value_address = &base_value;
    libh5n::option<word *> opt { base_value_address };
    auto addr = opt.unwrap();

    ASSERT_EQ(400, *addr);
}

TEST(option_test, size_test)
{
    libh5n::option<word> opt { 2038 };

    ASSERT_EQ(sizeof(word) * 2, sizeof(opt));
}

TEST(option_test, copy_test)
{
    libh5n::option<word> opt { 2038 };
    auto opt_2 = opt;

    ASSERT_EQ(opt_2.unwrap(), 2038);
}

TEST(option_test, return_option_int)
{
    // library::common::option<int> opt = get_opt_int();
    auto get_option_int = []() -> libh5n::option<int>
    {
        return 2038;
    };

    auto opt = get_option_int();

    ASSERT_EQ(opt.unwrap(), 2038);
}

TEST(option_test, return_option_none_test)
{
    auto get_option_int_none = []() -> libh5n::option<int>
    {
        return libh5n::option_none;
    };

    library::common::option<int> opt = get_option_int_none();

    ASSERT_EQ(opt.is_some(), false);
}

// trivial
struct foo
{
    int a;
};

// non-trivial
struct bar
{
    int a;

    bar() : a { 2038 }
    {
    }

    ~bar() {};
};

TEST(option_test, move_trivial_test)
{
    std::unique_ptr<foo> a = std::make_unique<foo>();
    // move
    option<std::unique_ptr<foo>> b = std::move(a);

    ASSERT_EQ(b.is_some(), true);
}

TEST(option_test, move_non_trivial_test)
{
    std::unique_ptr<bar> a = std::make_unique<bar>();
    // move
    option<std::unique_ptr<bar>> b = std::move(a);

    ASSERT_EQ(b.is_some(), true);
}

TEST(option_test, lazy_initialization_test)
{
    option<bar> a;

    ASSERT_EQ(a.is_some(), false);

    a = bar();

    ASSERT_EQ(a.is_some(), true);
}

TEST(option_test, operator_bool_test)
{
    auto get_option_int = []() -> libh5n::option<int>
    {
        return 2038;
    };

    auto get_option_none = []() -> libh5n::option<int>
    {
        return libh5n::option_none;
    };

    if (auto opt_int = get_option_int())
    {
        ASSERT_EQ(opt_int.unwrap(), 2038);
    }

    if (auto opt_none = get_option_none())
    {
        ASSERT_EQ(opt_none.is_some(), false);
    }
}

struct foo_has_constructor
{
    constexpr foo_has_constructor(int target_a, int target_b)
        : a { target_a }
        , b { target_b }
    {
    }

    const int a;
    const int b;
};

TEST(option_test, inplace_initialization_test)
{
    option<foo_has_constructor> opt(libh5n::option_in_place, 123, 456);

    ASSERT_EQ(opt.is_some(), true);
    ASSERT_EQ(opt.unwrap().a, 123);
    ASSERT_EQ(opt.unwrap().b, 456);
}

TEST(option_test, make_option_inplace_initialization_test)
{
    auto opt = libh5n::make_option_some<foo_has_constructor>(123, 456);

    ASSERT_EQ(opt.is_some(), true);
    ASSERT_EQ(opt.unwrap().a, 123);
    ASSERT_EQ(opt.unwrap().b, 456);
}

TEST(option_test, operator_indirect_test)
{
    option<int> opt { 2038 };

    ASSERT_EQ(opt.unwrap(), 2038);
    ASSERT_EQ(*opt, 2038);
}
