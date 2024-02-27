#include <gtest/gtest.h>

#include <kernel/capability/capability_component.hpp>

TEST(capability_slot_test, slot_data_get_element_test)
{
    auto slot = new kernel::capability_slot;

    slot->state.data.set_element(0, 0xdeadbeaf);

    ASSERT_EQ(slot->state.data.get_element(0), 0xdeadbeaf);
}

TEST(capability_slot_test, slot_data_out_of_range_test)
{
    auto slot = new kernel::capability_slot;

    slot->state.data.set_element(4, 0xdeadbeaf);

    ASSERT_NE(slot->state.data.get_element(4), 0xdeadbeaf);
}
