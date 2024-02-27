#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdio.h>

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_node.hpp>

class capability_node_test : public ::testing::Test
{
  protected:
    kernel::capability_node *node_root;
    kernel::capability_node *node_1;
    kernel::capability_node *node_2;

    kernel::capability_slot *slots_root;
    kernel::capability_slot *slots_1;
    kernel::capability_slot *slots_2;

    void SetUp() override
    {
        slots_root = new kernel::capability_slot[256];
        node_root = new kernel::capability_node(24, 8, slots_root);
    }

    void TearDown() override
    {
    }
};

TEST_F(capability_node_test, slot_data_get_element_test)
{
    auto slot = new kernel::capability_slot;

    slot->state.data.set_element(0, 0xdeadbeaf);

    ASSERT_EQ(slot->state.data.get_element(0), 0xdeadbeaf);
}

TEST_F(capability_node_test, slot_data_out_of_range_test)
{
    auto slot = new kernel::capability_slot;

    slot->state.data.set_element(4, 0xdeadbeaf);

    ASSERT_NE(slot->state.data.get_element(4), 0xdeadbeaf);
}

TEST_F(capability_node_test, root_retrieve_slot_min_test)
{
    slots_root[0].state.data.set_element(0, 0xdeadbeaf);

    auto slot = node_root->retrieve_slot(0);

    ASSERT_EQ(slot->state.data.get_element(0), 0xdeadbeaf);
}

TEST_F(capability_node_test, root_retrieve_slot_max_test)
{
    slots_root[255].state.data.set_element(0, 0xdeadbeaf);

    auto slot = node_root->retrieve_slot(255);

    ASSERT_EQ(slot->state.data.get_element(0), 0xdeadbeaf);
}

TEST_F(capability_node_test, root_retrieve_slot_out_of_range_test)
{
    auto slot = node_root->retrieve_slot(256);

    ASSERT_EQ(slot, nullptr);
}
