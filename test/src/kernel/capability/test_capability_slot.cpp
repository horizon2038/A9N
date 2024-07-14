#include <gtest/gtest.h>

#include <kernel/capability/capability_component.hpp>

TEST(capability_slot_test, slot_data_get_element_test)
{
    auto slot = new a9n::kernel::capability_slot {};

    slot->set_local_data(0, 0xdeadbeaf);
    ASSERT_EQ(slot->get_local_data(0), 0xdeadbeaf);
    delete slot;
}

TEST(capability_slot_test, slot_data_out_of_range_test)
{
    auto slot = new a9n::kernel::capability_slot {};

    // ASSERT_NE(slot->get_local_data(4), 0xdeadbeaf);
    delete slot;
}
