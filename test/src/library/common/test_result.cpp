#include <gtest/gtest.h>

#include <liba9n/result/result.hpp>
#include <iostream>

using word = uintmax_t;

enum class test_error : char
{
    error_a = 'a',
    error_b = 'b',
};

TEST(result_test, is_error_test)
{
    liba9n::result<int, test_error> r { test_error::error_a };
    ASSERT_EQ(r.is_error(), true);
    ASSERT_EQ(r.is_ok(), false);
}

TEST(result_test, is_ok_test)
{
    liba9n::result<int, test_error> r { 2038 };
    ASSERT_EQ(r.is_ok(), true);
    ASSERT_EQ(r.is_error(), false);
}

TEST(result_test, unwrap_test)
{
    liba9n::result<int, test_error> r { 2038 };
    ASSERT_EQ(r.unwrap(), 2038);
    ASSERT_NE(r.unwrap(), 128);
}

TEST(result_test, unwrap_error_test)
{
    liba9n::result<int, test_error> r { test_error::error_a };
    ASSERT_EQ(r.unwrap_error(), test_error::error_a);
}

TEST(result_test, pointer_test)
{
    int                               base_value         = 400;
    int                              *base_value_address = &base_value;
    liba9n::result<int *, test_error> r { base_value_address };
    auto                              addr = r.unwrap();

    ASSERT_EQ(400, *addr);
}

TEST(result_test, small_size_test)
{
    liba9n::result<word, test_error> r { 2038 };

    ASSERT_EQ(sizeof(word) * 2, sizeof(r));
}

TEST(result_test, copy_test)
{
    liba9n::result<int, test_error> r { 2038 };
    auto                            r_2 = r;

    ASSERT_EQ(r_2.unwrap(), 2038);
}

TEST(result_test, return_result_value_test)
{
    // liba9n::option<int> opt = get_opt_int();
    auto get_result = []() -> liba9n::result<int, test_error>
    {
        return 2038;
    };

    auto r = get_result();

    ASSERT_EQ(r.unwrap(), 2038);
}

TEST(result_test, return_result_error_test)
{
    auto get_result = []() -> liba9n::result<int, test_error>
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
    liba9n::result<std::unique_ptr<foo>, test_error> b = std::move(a);

    ASSERT_EQ(b.is_ok(), true);
}

TEST(result_test, move_non_trivial_test)
{
    std::unique_ptr<bar> a = std::make_unique<bar>();
    // move
    liba9n::result<std::unique_ptr<bar>, test_error> b = std::move(a);

    ASSERT_EQ(b.is_ok(), true);
}

TEST(result_test, operator_bool_test)
{
    auto get_result_value = []() -> liba9n::result<int, test_error>
    {
        return 2038;
    };

    auto get_result_error = []() -> liba9n::result<int, test_error>
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
    auto get_result_ok = []() -> liba9n::result<foo_has_constructor, test_error>
    {
        return liba9n::make_result_ok<foo_has_constructor, test_error>(123, 456);
    };

    auto r = get_result_ok();

    ASSERT_EQ(r.is_ok(), true);
    ASSERT_EQ(r.unwrap().a, 123);
    ASSERT_EQ(r.unwrap().b, 456);
}

TEST(result_test, is_trivially_destructible_test)
{
    liba9n::result<int, test_error> r { 2038 };

    ASSERT_EQ(std::is_trivially_destructible_v<decltype(r)>, true);
}

// monadic operations test
liba9n::result<int, test_error> increment(int x)
{
    return x + 1;
}

liba9n::result<int, test_error> square(int x)
{
    return x * x;
}

liba9n::result<int, test_error> fail(int x)
{
    return test_error::error_a;
}

TEST(result_test, and_then_success_test)
{
    liba9n::result<int, test_error> res { 42 };
    auto                            new_res = res.and_then(increment);

    ASSERT_TRUE(new_res.is_ok());
    EXPECT_EQ(new_res.unwrap(), 43);
}

TEST(result_test, and_then_error_test)
{
    liba9n::result<int, test_error> res     = test_error::error_a;
    auto                            new_res = res.and_then(increment);

    ASSERT_TRUE(new_res.is_error());
    EXPECT_EQ(new_res.unwrap_error(), test_error::error_a);
}

TEST(result_test, and_then_function_returns_error_test)
{
    liba9n::result<int, test_error> res { 42 };
    auto                            new_res = res.and_then(fail);

    ASSERT_TRUE(new_res.is_error());
    EXPECT_EQ(new_res.unwrap_error(), test_error::error_a);
}

TEST(result_test, and_then_ok_chain_test)
{
    liba9n::result<int, test_error> res { 1 };

    // clang-format off
    auto new_res = res
        .and_then(increment)
        .and_then(square)
        .and_then(square)
        .and_then([](int x)
        {
            return liba9n::result<int, test_error> { x * 5 };
        });
    // clang-format on

    ASSERT_TRUE(new_res);
    ASSERT_EQ(new_res.unwrap(), 80);
}

TEST(result_test, and_then_error_chain_test)
{
    liba9n::result<int, test_error> res { 1 };

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

TEST(result_test, and_then_string_error_test)
{
    liba9n::result<int, std::string> res { "not working!" };
    // clang-format off
    auto new_res = res
        .and_then([](int x) -> liba9n::result<int, std::string>
        {
            return "not working!";
        });
    // clang-format on

    ASSERT_FALSE(new_res);
    ASSERT_EQ(new_res.unwrap_error(), "not working!");
}

liba9n::result<int, test_error> fail_2(test_error &err)
{
    return 0;
}

TEST(result_test, or_else_success_test)
{
    liba9n::result<int, test_error> res { 42 };
    // 実行されないはず
    auto new_res = res.or_else(fail_2);

    ASSERT_TRUE(new_res.is_ok());
    ASSERT_EQ(new_res.unwrap(), 42);
}

liba9n::result<int, test_error> change_error(test_error &err)
{
    return test_error::error_b;
}

TEST(result_test, or_else_error_test)
{
    liba9n::result<int, test_error> res { test_error::error_a };
    auto                            new_res = res.or_else(change_error);

    ASSERT_FALSE(new_res);
    ASSERT_EQ(new_res.unwrap_error(), test_error::error_b);
}

liba9n::result<int, std::string> or_else_return_int(const std::string &e)
{
    return 203;
}

TEST(result_test, or_else_string_test)
{
    liba9n::result<int, std::string> res { "hello, world!" };

    auto new_res = res.or_else(or_else_return_int).unwrap();

    ASSERT_EQ(203, new_res);
    ASSERT_TRUE(new_res);
}

TEST(result_test, transform_success_test)
{
    liba9n::result<int, std::string> res { 1 };
    auto                             new_res = res.transform(
        [](int x)
        {
            return x * 2;
        }
    );

    ASSERT_TRUE(new_res);
    ASSERT_EQ(new_res.unwrap(), 2);
}

TEST(result_test, transform_error_test)
{
    liba9n::result<int, std::string> res { "result : error!" };
    auto                             new_res = res.transform(
        [](int x)
        {
            return x;
        }
    );

    ASSERT_FALSE(new_res);
    ASSERT_EQ(new_res.unwrap_error(), "result : error!");
}

TEST(result_test, transform_chain_test)
{
    // clang-format off

    liba9n::result<int, std::string> res { 4 };
    auto new_res = res
        .transform(
            [](int x)
            {
                return x * x;
            }
        )
        .and_then(
            [](int x) -> liba9n::result<int, std::string>
            {
                return std::string("failed!");
            }
        )
        .or_else(
            [](std::string e) -> liba9n::result<int, std::string>
            {
                return std::string("failed (modified) !");
            }
        );

        ASSERT_FALSE(new_res);
        ASSERT_EQ(new_res.unwrap_error(), "failed (modified) !");

    // clang-format on
}

TEST(result_test, unwrap_or_t_test)
{
    liba9n::result<int, std::string> res { 42 };

    ASSERT_TRUE(res);
    ASSERT_EQ(res.unwrap_or(0), 42);
}

TEST(result_test, unwrap_or_e_test)
{
    liba9n::result<int, std::string> res { std::string("hello, world!") };

    ASSERT_FALSE(res);
    ASSERT_EQ(res.unwrap_or(0), 0);
}

TEST(result_test, unwrap_or_error_t_test)
{
    liba9n::result<int, std::string> res { 42 };

    ASSERT_TRUE(res);
    ASSERT_EQ(
        res.unwrap_error_or(std::string("hello, world!")),
        std::string("hello, world!")
    );
}

TEST(result_test, unwrap_or_error_e_test)
{
    liba9n::result<int, std::string> res { std::string("hello, world!") };

    ASSERT_FALSE(res);
    ASSERT_EQ(
        res.unwrap_error_or(std::string("goodnight, world!")),
        std::string("hello, world!")
    );
}

liba9n::result<int, std::string> safe_divide(int a, int b)
{
    if (b == 0)
    {
        return "divide by zero";
    }

    if (a % b != 0)
    {
        return "out of domain";
    }

    return a / b;
}

TEST(result_test, safe_divide_test)
{
    auto res = safe_divide(10, 2);
    ASSERT_EQ(res.unwrap(), 5);

    res = safe_divide(10, 3);
    ASSERT_EQ(res.unwrap_error(), "out of domain");

    res = safe_divide(10, 0);
    ASSERT_EQ(res.unwrap_error(), "divide by zero");
}

// result<void, E> test

TEST(result_void_rest, is_error_test)
{
    liba9n::result<void, std::string> res { std::string("hello, world!") };
    ASSERT_TRUE(res.is_error());
}

TEST(result_void_test, is_ok_test)
{
    liba9n::result<void, std::string> res {};
    ASSERT_TRUE(res.is_ok());
}

TEST(result_void_test, chain_test)
{
    liba9n::result<void, std::string> res {};

    // clang-format off
    auto res_2 = res.and_then(
        []() -> liba9n::result<void, std::string>
        {
            return "text";
        }
    )
    .or_else(
        [](const std::string &e) -> liba9n::result<void, std::string>
        {
            std::cout << e << std::endl;
            return {};
        }
    );

    ASSERT_TRUE(res_2.is_ok());
    // clang-format on
}

TEST(result_void_test, transform_test)
{
    liba9n::result<void, std::string> res {};
    // clang-format off
    auto new_res = res
        .transform(
            []()
            {
                std::cout << "hello,";
            }
        )
        .transform(
            []()
            {
                std::cout << "world!";
            }
        );
    // clang-format on
}
