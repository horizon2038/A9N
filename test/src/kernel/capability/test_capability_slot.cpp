#include <gtest/gtest.h>

#include <kernel/capability/capability_component.hpp>

TEST(capability_slot_test, slot_data_get_element_test)
{
    a9n::kernel::capability_slot slot {};

    slot.set_local_data<0>(0xdeadbeaf);
    ASSERT_EQ(slot.get_local_data<0>(), 0xdeadbeaf);
}

TEST(capability_slot_Test, slot_size_test)
{
    std::cout << "slot_size : " << sizeof(a9n::kernel::capability_slot)
              << std::endl;
}
