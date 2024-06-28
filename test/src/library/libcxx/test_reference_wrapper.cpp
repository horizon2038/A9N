#include <gtest/gtest.h>

#include <liba9n/libcxx/functional>

TEST(reference_wrapper_test, test_reference)
{
    int a = 7;
    liba9n::std::reference_wrapper<int> rw_a = liba9n::std::ref(a);

    int &r_rw_a = rw_a.get();

    ASSERT_EQ(r_rw_a, 7);

    r_rw_a = 10;

    ASSERT_EQ(a, 10);
}
