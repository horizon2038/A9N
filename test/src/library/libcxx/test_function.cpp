#include <gtest/gtest.h>

#include <library/libcxx/functional>

TEST(function_test, invoke_lambda_test)
{
    liba9n::std::function<int(int, int)> func(
        [](int a, int b)
        {
            return a + b;
        }
    );
    ASSERT_EQ(func(2, 3), 5);
}

struct multiplier
{
    int operator()(int a, int b) const
    {
        return a * b;
    }
};

TEST(function_test, invoke_function_object_test)
{
    liba9n::std::function<int(int, int)> func(multiplier {});
    ASSERT_EQ(func(4, 5), 20);
}

int subtract(int a, int b)
{
    return a - b;
}

TEST(function_test, invoke_free_function_test)
{
    liba9n::std::function<int(int, int)> func(subtract);
    ASSERT_EQ(func(9, 4), 5);
}
