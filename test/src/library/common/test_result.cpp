#include <gtest/gtest.h>

#include <library/common/result.hpp>

namespace libh5n = library::common;
using word = uintmax_t;

enum class test_error : char
{
    error_a = 'a',
    error_b = 'b',
};

TEST(result_test, has_error_test)
{
    library::common::result<int, test_error> r { test_error::error_a };
    ASSERT_EQ(r.has_error(), true);
    ASSERT_EQ(r.has_value(), false);
}

TEST(result_test, has_value_test)
{
    library::common::result<int, test_error> r { 2038 };
    ASSERT_EQ(r.has_value(), true);
    ASSERT_EQ(r.has_error(), false);
}

TEST(result_test, unwrap_test)
{
    library::common::result<int, test_error> r { 2038 };
    ASSERT_EQ(r.unwrap(), 2038);
    ASSERT_NE(r.unwrap(), 128);
}

TEST(result_test, unwrap_error_test)
{
    library::common::result<int, test_error> r { test_error::error_a };
    ASSERT_EQ(r.unwrap_error(), test_error::error_a);
}

TEST(result_test, pointer_test)
{
    int base_value = 400;
    int *base_value_address = &base_value;
    library::common::result<int *, test_error> r { base_value_address };
    auto addr = r.unwrap();

    ASSERT_EQ(400, *addr);
}

TEST(result_test, small_size_test)
{
    library::common::result<word, test_error> r { 2038 };

    ASSERT_EQ(sizeof(word) * 2, sizeof(r));
}

TEST(result_test, copy_test)
{
    library::common::result<int, test_error> r { 2038 };
    auto r_2 = r;

    ASSERT_EQ(r_2.unwrap(), 2038);
}

TEST(result_test, return_result_value_test)
{
    // library::common::option<int> opt = get_opt_int();
    auto get_result = []() -> library::common::result<int, test_error>
    {
        return 2038;
    };

    auto r = get_result();

    ASSERT_EQ(r.unwrap(), 2038);
}

TEST(result_test, return_result_error_test)
{
    auto get_result = []() -> library::common::result<int, test_error>
    {
        return test_error::error_a;
    };

    auto r = get_result();

    ASSERT_EQ(r.unwrap_error(), test_error::error_a);
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

TEST(result_test, move_trivial_test)
{
    std::unique_ptr<foo> a = std::make_unique<foo>();
    // move
    library::common::result<std::unique_ptr<foo>, test_error> b = std::move(a);

    ASSERT_EQ(b.has_value(), true);
}

TEST(result_test, move_non_trivial_test)
{
    std::unique_ptr<bar> a = std::make_unique<bar>();
    // move
    library::common::result<std::unique_ptr<bar>, test_error> b = std::move(a);

    ASSERT_EQ(b.has_value(), true);
}

TEST(result_test, operator_bool_test)
{
    auto get_result_value = []() -> library::common::result<int, test_error>
    {
        return 2038;
    };

    auto get_result_error = []() -> library::common::result<int, test_error>
    {
        return test_error::error_a;
    };

    if (auto r = get_result_value())
    {
        ASSERT_EQ(r.unwrap(), 2038);
    }

    if (auto r = get_result_error())
    {
        ASSERT_EQ(r.unwrap_error(), test_error::error_a);
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

TEST(result_test, make_result_ok_inplace_test)
{
    auto get_result_ok
        = []() -> library::common::result<foo_has_constructor, test_error>
    {
        return library::common::make_result_ok<foo_has_constructor, test_error>(
            123,
            456
        );
    };

    auto r = get_result_ok();

    ASSERT_EQ(r.has_value(), true);
    ASSERT_EQ(r.unwrap().a, 123);
    ASSERT_EQ(r.unwrap().b, 456);
}

TEST(result_test, is_trivially_destructible_test)
{
    library::common::result<int, test_error> r { 2038 };

    ASSERT_EQ(std::is_trivially_destructible_v<decltype(r)>, true);
}

// monadic operations test
library::common::result<int, test_error> increment(int x)
{
    return x + 1;
}

library::common::result<int, test_error> square(int x)
{
    return x * x;
}

library::common::result<int, test_error> fail(int x)
{
    return test_error::error_a;
}

TEST(result_test, and_then_success_test)
{
    library::common::result<int, test_error> res { 42 };
    auto new_res = res.and_then(increment);

    ASSERT_TRUE(new_res.has_value());
    EXPECT_EQ(new_res.unwrap(), 43);
}

TEST(result_test, and_then_error_test)
{
    library::common::result<int, test_error> res = test_error::error_a;
    auto new_res = res.and_then(increment);

    ASSERT_TRUE(new_res.has_error());
    EXPECT_EQ(new_res.unwrap_error(), test_error::error_a);
}

TEST(result_test, and_then_function_returns_error_test)
{
    library::common::result<int, test_error> res { 42 };
    auto new_res = res.and_then(fail);

    ASSERT_TRUE(new_res.has_error());
    EXPECT_EQ(new_res.unwrap_error(), test_error::error_a);
}

TEST(result_test, and_then_ok_chain_test)
{
    library::common::result<int, test_error> res { 1 };

    // clang-format off
    auto new_res = res
        .and_then(increment)
        .and_then(square)
        .and_then(square);
    // clang-format on

    ASSERT_TRUE(new_res);
    ASSERT_EQ(new_res.unwrap(), 16);
}

TEST(result_test, and_then_error_chain_test)
{
    library::common::result<int, test_error> res { 1 };

    // clang-format off
    auto new_res = res
        .and_then(increment)
        .and_then(square)
        .and_then(square)
        .and_then(fail);
    // clang-format on

    ASSERT_FALSE(new_res);
    ASSERT_EQ(new_res.unwrap_error(), test_error::error_a);
}
