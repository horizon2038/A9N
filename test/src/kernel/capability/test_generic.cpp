#include <gtest/gtest.h>

#include <kernel/capability/generic.hpp>

class generic_test : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        return;
    }

    void TearDown() override
    {
        return;
    }
};

TEST_F(generic_test, generic_convert_test)
{
    kernel::generic g;

    SUCCEED();
}
