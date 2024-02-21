#include <gtest/gtest.h>

bool is_even(int x)
{
    return x % 2 == 0;
}

TEST(IsEvenTest, Negative)
{
    EXPECT_FALSE(is_even(-1));
    EXPECT_TRUE(is_even(-2));
}
