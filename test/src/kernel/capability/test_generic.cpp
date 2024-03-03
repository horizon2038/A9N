#include <gtest/gtest.h>

#include <kernel/capability/generic.hpp>
#include <kernel/capability/capability_node.hpp>
#include <kernel/capability/capability_component.hpp>
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
        this->root_node_slot = new kernel::capability_slot[256];
        this->root_node = new kernel::capability_node(56, 8, root_node_slot);
        this->root_slot.component = root_node;
        return;
    }

    void TearDown() override
    {
        delete[] root_node_slot;
        delete root_node;
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

inline bool is_device(library::common::word flags)
{
    return (flags >> 7) & 1;
}

inline library::common::word calculate_size(library::common::word flags)
{
    library::common::word size_mask = (1 << (7)) - 1;
    return static_cast<library::common::word>(1) << (flags & size_mask);
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
    buffer.set_element(7,
                       0); // target capability_node descriptor
    buffer.set_element(8, 1); // target index

    auto e
        = generic_slot->component->execute(generic_slot, &root_slot, &buffer);

    auto watermark = generic_slot->data.get_element(2);
    ASSERT_EQ(watermark, (0x1000 + 0x100));

    auto child = root_slot.component->retrieve_slot(1);
    auto child_size = calculate_size(child->data.get_element(1));
    printf("element_1 : %llu\n", child->data.get_element(1));
    printf(
        "child_slot_address : 0x%llx",
        reinterpret_cast<library::common::word>(child)
    );
    ASSERT_EQ(0x100, child_size);
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
    // 6 : target node descriptor
    // 7 : target node depth
    // 8 : target node index
    buffer.set_element(0, 0);
    buffer.set_element(1, library::common::WORD_BITS);
    buffer.set_element(2, 0); // generic::CONVERT
    buffer.set_element(3, 1); // generic
    buffer.set_element(4, 10); // 2^8 = 256 byte
    buffer.set_element(5, 1); // count-1
    buffer.set_element(6, 0); // target capability_node descriptor
    buffer.set_element(7,
                       0); // target capability_node descriptor
    buffer.set_element(8, 1); // target index

    g.execute(generic_slot, &root_slot, &buffer);

    auto watermark = generic_slot->data.get_element(2);
    ASSERT_EQ(watermark, (0x1000 + 0x400));
}
