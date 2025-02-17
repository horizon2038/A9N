#include <gtest/gtest.h>

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_node.hpp>
#include <kernel/capability/generic.hpp>
#include <kernel/ipc/ipc_buffer.hpp>
#include <kernel/process/process.hpp>

#include <kernel/types.hpp>
#include <liba9n/capability/generic_operation.hpp>

class generic_test : public ::testing::Test
{
  protected:
    a9n::kernel::process               current_process;
    a9n::kernel::capability_slot      *root_node_slot;
    a9n::kernel::capability_component *root_node;

    void SetUp() override
    {
        this->root_node_slot = new a9n::kernel::capability_slot[256];
        this->root_node = new a9n::kernel::capability_node(56, 8, root_node_slot);
        this->current_process.root_slot.component = root_node;
        return;
    }

    void TearDown() override
    {
        delete[] root_node_slot;
        delete root_node;
        return;
    }
};

a9n::word calculate_generic_flags(bool is_device, a9n::word size_bits)
{
    a9n::word generic_flags {};

    generic_flags       |= (is_device << 7);
    auto size_bits_mask  = (1 << 7) - 1;
    generic_flags       |= (size_bits & size_bits_mask);

    return generic_flags;
}

inline bool is_device(a9n::word flags)
{
    return (flags >> 7) & 1;
}

inline a9n::word calculate_size(a9n::word flags)
{
    a9n::word size_mask = (1 << (7)) - 1;
    return static_cast<a9n::word>(1) << (flags & size_mask);
}

TEST_F(generic_test, generic_convert_generic_size_8_watermark_test)
{
    a9n::kernel::generic    g;
    a9n::kernel::ipc_buffer buffer;

    auto generic_slot
        = current_process.root_slot.component->retrieve_slot(0).unwrap();
    generic_slot->component = &g;
    generic_slot->set_local_data(0, 0x1000);
    auto generic_flags = calculate_generic_flags(0, 0xc);
    generic_slot->set_local_data(1, generic_flags);
    generic_slot->set_local_data(2, 0x1000);

    // buffer
    // 0 : operation_type
    // 1 : type
    // 2 : size
    // 3 : count
    // buffer.set_message(0, 0);
    // buffer.set_message(1, a9n::WORD_BITS);
    // buffer.set_message(0, 0); // generic::CONVERT

    buffer.message_tag = 0;
    buffer.set_message(0, 1); // generic
    buffer.set_message(1, 8); // 2^8 = 256 byte
    buffer.set_message(2, 1); // count-1
    buffer.set_message(3, 0); // target capability_node descriptor
    buffer.set_message(4, 0); // target apability_node depth
    buffer.set_message(5, 1); // target index

    current_process.buffer = &buffer;

    auto e = generic_slot->component->execute(current_process, *generic_slot);

    auto watermark = generic_slot->get_local_data(2);
    ASSERT_EQ(watermark, (0x1000 + 0x100));

    auto child = current_process.root_slot.component->retrieve_slot(1).unwrap();
    auto child_size = calculate_size(child->get_local_data(1));
    ASSERT_EQ(0x100, child_size);
}

TEST_F(generic_test, generic_convert_generic_size_10_watermark_test)
{
    a9n::kernel::generic    g;
    a9n::kernel::ipc_buffer buffer;

    auto generic_slot
        = current_process.root_slot.component->retrieve_slot(0).unwrap();
    generic_slot->component = &g;
    generic_slot->set_local_data(0, 0x1000);
    auto generic_flags = calculate_generic_flags(0, 0xc);
    generic_slot->set_local_data(1, generic_flags);
    generic_slot->set_local_data(2, 0x1000);

    // buffer
    // 0 : operation_type
    // 1 : type
    // 2 : size
    // 3 : count
    // 4 : target node descriptor
    // 5 : target node depth
    // 6 : target node index

    // buffer.set_element(0, 0);
    // buffer.set_element(1, a9n::WORD_BITS);
    // buffer.set_message(0, 0); // generic::CONVERT
    //
    buffer.message_tag = 0;
    buffer.set_message(0, 1);  // generic
    buffer.set_message(1, 10); // 2^10 = 1 kbyte
    buffer.set_message(2, 1);  // count
    buffer.set_message(3, 0);  // target capability_node descriptor
    buffer.set_message(4, 0);  // target capability_node depth
    buffer.set_message(5, 1);  // target index
    current_process.buffer = &buffer;

    auto e = generic_slot->component->execute(current_process, *generic_slot);

    auto watermark = generic_slot->get_local_data(2);
    ASSERT_EQ(watermark, (0x1000 + 0x400));

    auto child = current_process.root_slot.component->retrieve_slot(1).unwrap();
    auto child_size = calculate_size(child->get_local_data(1));
    ASSERT_EQ(0x400, child_size);
}
