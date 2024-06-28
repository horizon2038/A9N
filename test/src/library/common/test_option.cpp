#include <gtest/gtest.h>

#include <liba9n/common/option.hpp>

#include <liba9n/common/types.hpp>

#include <liba9n/libcxx/functional>

TEST(option_test, is_some_test)
{
    liba9n::common::option opt { 0x40 };

    ASSERT_EQ(true, opt.is_some());
}

TEST(option_test, unwrap_test)
{
    liba9n::common::option opt { 0xdeadbeaf };

    ASSERT_EQ(0xdeadbeaf, opt.unwrap());
}

TEST(option_test, none_test)
{
    liba9n::common::option opt = liba9n::common::option_none;

    ASSERT_EQ(false, opt.is_some());
}

TEST(option_test, pointer_test)
{
    a9n::word base_value = 400;
    a9n::word *base_value_address = &base_value;
    liba9n::common::option<a9n::word *> opt { base_value_address };
    auto addr = opt.unwrap();

    ASSERT_EQ(400, *addr);
}

TEST(option_test, size_test)
{
    liba9n::common::option<a9n::word> opt { 2038 };

    ASSERT_EQ(sizeof(a9n::word) * 2, sizeof(opt));
}

TEST(option_test, copy_test)
{
    liba9n::common::option<a9n::word> opt { 2038 };
    auto opt_2 = opt;

    ASSERT_EQ(opt_2.unwrap(), 2038);
}

TEST(option_test, return_option_int)
{
    // liba9n::common::option<int> opt = get_opt_int();
    auto get_option_int = []() -> liba9n::common::option<int>
    {
        return 2038;
    };

    auto opt = get_option_int();

    ASSERT_EQ(opt.unwrap(), 2038);
}

TEST(option_test, return_option_none_test)
{
    auto get_option_int_none = []() -> liba9n::common::option<int>
    {
        return liba9n::common::option_none;
    };

    liba9n::common::option<int> opt = get_option_int_none();

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
    liba9n::common::option<std::unique_ptr<foo>> b = std::move(a);

    ASSERT_EQ(b.is_some(), true);
}

TEST(option_test, move_non_trivial_test)
{
    std::unique_ptr<bar> a = std::make_unique<bar>();
    // move
    liba9n::common::option<std::unique_ptr<bar>> b = std::move(a);

    ASSERT_EQ(b.is_some(), true);
}

TEST(option_test, lazy_initialization_test)
{
    liba9n::common::option<bar> a;

    ASSERT_EQ(a.is_some(), false);

    a = bar();

    ASSERT_EQ(a.is_some(), true);
}

TEST(option_test, operator_bool_test)
{
    auto get_option_int = []() -> liba9n::common::option<int>
    {
        return 2038;
    };

    auto get_option_none = []() -> liba9n::common::option<int>
    {
        return liba9n::common::option_none;
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
    liba9n::common::option<foo_has_constructor> opt(
        liba9n::common::option_in_place,
        123,
        456
    );

    ASSERT_EQ(opt.is_some(), true);
    ASSERT_EQ(opt.unwrap().a, 123);
    ASSERT_EQ(opt.unwrap().b, 456);
}

TEST(option_test, make_option_inplace_initialization_test)
{
    auto opt = liba9n::common::make_option_some<foo_has_constructor>(123, 456);

    ASSERT_EQ(opt.is_some(), true);
    ASSERT_EQ(opt.unwrap().a, 123);
    ASSERT_EQ(opt.unwrap().b, 456);
}

TEST(option_test, operator_indirect_test)
{
    liba9n::common::option<int> opt { 2038 };

    ASSERT_EQ(opt.unwrap(), 2038);
    ASSERT_EQ(*opt, 2038);
}

TEST(option_test, option_chain_test)
{
    liba9n::common::option<int> opt { 2038 };
    // clang-format off
    auto opt_2 = opt
        .and_then([](int x) -> liba9n::common::option<int>
        {
            return x + 10;
        })
         .and_then([](int x) -> liba9n::common::option<int>
        {
            std::cout << x << std::endl;
            return liba9n::common::option_none;
        });
    // clang-format on
    // ASSERT_EQ(opt_2.unwrap(), 2048);
    ASSERT_TRUE(opt_2.is_none());
}

TEST(option_test, option_equality_test)
{
    liba9n::common::option<int> opt { 42 };
    liba9n::common::option<int> opt_same { 42 };
    liba9n::common::option<int> opt_diff { 32 };
    liba9n::common::option<int> opt_none {};

    ASSERT_TRUE(opt == opt_same);
    ASSERT_FALSE(opt == opt_diff);
    ASSERT_FALSE(opt == opt_none);
}

static int ref_value { 20 };

liba9n::common::option<liba9n::std::reference_wrapper<int>> return_ref()
{
    return liba9n::std::ref(ref_value);
}

TEST(option_test, option_reference_test)
{
    auto opt = return_ref();
    ASSERT_EQ(opt.unwrap(), ref_value);
}
