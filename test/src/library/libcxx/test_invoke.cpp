#include <gtest/gtest.h>

#include <library/libcxx/functional>

class test_class
{
  public:
    int member_function(int x) const
    {
        return x + 1;
    }

    static int static_member_function(int x)
    {
        return x + 2;
    }
};

int free_function(int x)
{
    return x + 3;
}

TEST(invoke_test, free_function_test)
{
    EXPECT_EQ(library::std::invoke(free_function, 1), 4);
}

TEST(invoke_test, static_member_function_test)
{
    EXPECT_EQ(library::std::invoke(&test_class::static_member_function, 1), 3);
}

TEST(invoke_test, member_function_test)
{
    test_class obj;
    EXPECT_EQ(library::std::invoke(&test_class::member_function, obj, 1), 2);
}

TEST(invoke_test, lambda_test)
{
    auto lambda = [](int x)
    {
        return x + 4;
    };
    EXPECT_EQ(library::std::invoke(lambda, 1), 5);
}
