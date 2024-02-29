#include "kernel/capability/capability_component.hpp"
#include <gtest/gtest.h>

#include <kernel/capability/generic.hpp>
#include <kernel/capability/capability_node.hpp>
#include <kernel/ipc/message_buffer.hpp>

#include <library/capability/generic_operation.hpp>
#include <library/common/types.hpp>

class generic_test : public ::testing::Test
{
  protected:
    kernel::capability_slot root_slot;
    kernel::capability_slot *root_node_slot;
    kernel::capability_component *root_node;

    void SetUp() override
    {
        root_node_slot = new kernel::capability_slot[256];
        root_node = new kernel::capability_node(56, 8, root_node_slot);
        root_slot.component = root_node;
        return;
    }

    void TearDown() override
    {
        return;
    }
};

library::common::word
    calculate_generic_flags(bool is_device, library::common::word size_bits)
{
    library::common::word generic_flags {};

    generic_flags |= (is_device << 7);
    auto size_bits_mask = (1 << 7) - 1;
    generic_flags |= (size_bits & size_bits_mask);

    return generic_flags;
}

TEST_F(generic_test, generic_convert_generic_size_8_watermark_test)
{
    kernel::generic g;
    kernel::message_buffer buffer;

    auto generic_slot = root_slot.component->retrieve_slot(0);
    generic_slot->component = &g;
    generic_slot->data.set_element(0, 0x1000);
    auto generic_flags = calculate_generic_flags(0, 0xc);
    generic_slot->data.set_element(1, generic_flags);
    generic_slot->data.set_element(2, 0x1000);

    // buffer
    // 0 : descriptor (this) [reserved]
    // 1 : descriptor_depth  [reserved]
    // 2 : operation_type
    // 3 : type
    // 4 : size
    // 5 : count
    buffer.set_element(0, 0);
    buffer.set_element(1, library::common::WORD_BITS);
    buffer.set_element(2, 0); // generic::CONVERT
    buffer.set_element(3, 1); // generic
    buffer.set_element(4, 8); // 2^8 = 256 byte
    buffer.set_element(5, 1); // count-1
    buffer.set_element(6, 0); // target capability_node descriptor
    buffer.set_element(
        7,
        library::common::WORD_BITS
    ); // target capability_node descriptor
    buffer.set_element(8, 1); // target index

    g.execute(generic_slot, &root_slot, &buffer);

    auto watermark = generic_slot->data.get_element(2);
    ASSERT_EQ(watermark, (0x1000 + 0x100));
}

TEST_F(generic_test, generic_convert_generic_size_10_watermark_test)
{
    kernel::generic g;
    kernel::message_buffer buffer;

    auto generic_slot = root_slot.component->retrieve_slot(0);
    generic_slot->component = &g;
    generic_slot->data.set_element(0, 0x1000);
    auto generic_flags = calculate_generic_flags(0, 0xc);
    generic_slot->data.set_element(1, generic_flags);
    generic_slot->data.set_element(2, 0x1000);

    // buffer
    // 0 : descriptor (this) [reserved]
    // 1 : descriptor_depth  [reserved]
    // 2 : operation_type
    // 3 : type
    // 4 : size
    // 5 : count
    buffer.set_element(0, 0);
    buffer.set_element(1, library::common::WORD_BITS);
    buffer.set_element(2, 0); // generic::CONVERT
    buffer.set_element(3, 1); // generic
    buffer.set_element(4, 10); // 2^8 = 256 byte
    buffer.set_element(5, 1); // count-1
    buffer.set_element(6, 0); // target capability_node descriptor
    buffer.set_element(
        7,
        library::common::WORD_BITS
    ); // target capability_node descriptor
    buffer.set_element(8, 1); // target index

    g.execute(generic_slot, &root_slot, &buffer);

    auto watermark = generic_slot->data.get_element(2);
    ASSERT_EQ(watermark, (0x1000 + 0x400));
}
