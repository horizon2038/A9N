#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_node.hpp>

#include <kernel/types.hpp>

class capability_node_test : public ::testing::Test
{
  protected:
    a9n::kernel::capability_node *node_root;
    a9n::kernel::capability_node *node_1;
    a9n::kernel::capability_node *node_2;

    a9n::kernel::capability_slot *slots_root;
    a9n::kernel::capability_slot *slots_1;
    a9n::kernel::capability_slot *slots_2;

    constexpr static a9n::word NODE_ROOT_RADIX = 8;
    constexpr static a9n::word NODE_1_RADIX    = 12;
    constexpr static a9n::word NODE_2_RADIX    = 12;

    constexpr static a9n::word NODE_ROOT_IGNORE = 24;
    constexpr static a9n::word NODE_1_IGNORE    = 4;
    constexpr static a9n::word NODE_2_IGNORE    = 4;

    constexpr static a9n::word NODE_ROOT_ALLBITS
        = (NODE_ROOT_RADIX + NODE_ROOT_IGNORE);
    static_assert(NODE_ROOT_ALLBITS == 32, "NODE_ROOT_ALLBITS");

    constexpr static a9n::word NODE_1_ALLBITS
        = (NODE_ROOT_ALLBITS + NODE_1_RADIX + NODE_1_IGNORE);
    static_assert(NODE_1_ALLBITS == 48, "NODE_1_ALLBITS");

    constexpr static a9n::word NODE_2_ALLBITS
        = (NODE_1_ALLBITS + NODE_2_RADIX + NODE_1_IGNORE);
    static_assert(NODE_2_ALLBITS == 64, "NODE_2_ALLBITS");

    void SetUp() override
    {
        // node_root
        slots_root = new a9n::kernel::capability_slot[1 << NODE_ROOT_RADIX];
        node_root  = new a9n::kernel::capability_node(
            NODE_ROOT_IGNORE,
            NODE_ROOT_RADIX,
            slots_root
        );

        // node_1
        slots_1 = new a9n::kernel::capability_slot[1 << NODE_1_RADIX];
        node_1  = new a9n::kernel::capability_node(
            NODE_1_IGNORE,
            NODE_1_RADIX,
            slots_1
        );
        slots_root[0].component = node_1;

        // node_2
        slots_2 = new a9n::kernel::capability_slot[1 << NODE_2_RADIX];
        node_2  = new a9n::kernel::capability_node(
            NODE_2_IGNORE,
            NODE_2_RADIX,
            slots_2
        );
        slots_1[0].component = node_2;
    }

    void TearDown() override
    {
        delete node_root;
        delete node_1;
        delete node_2;

        delete[] slots_root;
        delete[] slots_1;
        delete[] slots_2;
    }
};

TEST_F(capability_node_test, retrieve_slot_index_min_test)
{
    slots_root[0].set_local_data(0, 0xdeadbeaf);

    auto slot = node_root->retrieve_slot(0);

    ASSERT_EQ(slot->get_local_data(0), 0xdeadbeaf);
}

TEST_F(capability_node_test, retrieve_slot_index_max_test)
{
    slots_root[255].set_local_data(0, 0xdeadbeaf);

    auto slot = node_root->retrieve_slot(255);

    ASSERT_EQ(slot->get_local_data(0), 0xdeadbeaf);
}

TEST_F(capability_node_test, retrieve_slot_index_out_of_range_test)
{
    auto slot = node_root->retrieve_slot(256);

    ASSERT_EQ(slot, nullptr);
}

TEST_F(capability_node_test, traverse_slot_index_min_depth_root_test)
{
    liba9n::capability::capability_descriptor descriptor = 0x0000000000000000;

    auto slot = node_root->traverse_slot(descriptor, 32, 0);

    ASSERT_EQ(slot, &(slots_root[0]));
}

TEST_F(capability_node_test, traverse_slot_index_max_depth_root_test)
{
    liba9n::capability::capability_descriptor descriptor = 0x000000ff00000000;

    auto slot = node_root->traverse_slot(descriptor, NODE_ROOT_ALLBITS, 0);

    ASSERT_EQ(slot, &(slots_root[(1 << NODE_ROOT_RADIX) - 1]));
}

TEST_F(capability_node_test, traverse_slot_index_min_depth_1_test)
{
    liba9n::capability::capability_descriptor descriptor = 0x0000000000000000;

    auto slot = node_root->traverse_slot(descriptor, NODE_1_ALLBITS, 0);

    ASSERT_EQ(slot, &(slots_1[0]));
}

TEST_F(capability_node_test, traverse_slot_index_max_depth_1_test)
{
    liba9n::capability::capability_descriptor descriptor = 0x000000000fff0000;

    auto slot = node_root->traverse_slot(descriptor, NODE_1_ALLBITS, 0);

    ASSERT_EQ(slot, &(slots_1[(1 << NODE_1_RADIX) - 1]));
}

TEST_F(capability_node_test, traverse_slot_index_min_depth_2_test)
{
    liba9n::capability::capability_descriptor descriptor = 0x0000000000000000;

    auto slot = node_root->traverse_slot(descriptor, NODE_2_ALLBITS, 0);

    ASSERT_EQ(slot, &(slots_2[0]));
}

TEST_F(capability_node_test, traverse_slot_index_max_depth_2_test)
{
    liba9n::capability::capability_descriptor descriptor = 0x0000000000000fff;

    auto slot = node_root->traverse_slot(descriptor, NODE_2_ALLBITS, 0);

    ASSERT_EQ(slot, &(slots_2[(1 << NODE_2_RADIX) - 1]));
}
