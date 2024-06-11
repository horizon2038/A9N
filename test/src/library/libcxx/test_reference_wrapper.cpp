#include <gtest/gtest.h>

#include <library/libcxx/functional>

TEST(reference_wrapper_test, test_reference)
{
    int a = 7;
    library::std::reference_wrapper<int> rw_a = library::std::ref(a);

    int &r_rw_a = rw_a.get();

    ASSERT_EQ(r_rw_a, 7);

    r_rw_a = 10;

    ASSERT_EQ(a, 10);
}
