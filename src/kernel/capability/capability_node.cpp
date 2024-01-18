#include "library/capability/capability_descriptor.hpp"
#include "library/common/types.hpp"
#include <kernel/capability/capability_node.hpp>
#include <kernel/utility/logger.hpp>

namespace kernel
{
    capability_node::capability_node(
        common::word initial_ignore_bits,
        common::word initial_radix_bits,
        capability_entry *initial_capability_slots
    )
        : capability_slots(initial_capability_slots)
        , ignore_bits(initial_ignore_bits)
        , radix_bits(initial_radix_bits)
    {
    }

    common::error capability_node::capability_node::execute(
        message_buffer *buffer,
        capability_entry *stored_entry
    )
    {
        kernel::utility::logger::printk("execute : node\n");

        // 1. decode operation
        // 2. dispatch operation
        // 3.run operation
        return 1;
    };

    common::error capability_node::revoke()
    {
        return 0;
    };

    capability_type capability_node::type()
    {
        return capability_type::NODE;
    }

    // recursively explores entries. this is a composite pattern that allows
    // handling single and multiple capabilities with the same interface.
    capability_entry *capability_node::traverse_entry(
        library::capability::capability_descriptor descriptor,
        common::word descriptor_max_bits, // usually WORD_BITS is used.
        common::word descriptor_used_bits
    )
    {
        auto entry = lookup_entry(descriptor, descriptor_used_bits);

        if (entry->capability_pointer->type() != capability_type::NODE)
        {
            return entry;
        }

        if (descriptor_used_bits == library::common::WORD_BITS)
        {
            return entry;
        }

        auto new_used_bits = calculate_used_bits(descriptor_used_bits);
        if (new_used_bits == descriptor_max_bits)
        {
            return entry;
        }
        return entry->capability_pointer
            ->traverse_entry(descriptor, descriptor_max_bits, new_used_bits);
    }

    capability_entry *capability_node::lookup_entry(
        library::capability::capability_descriptor descriptor,
        common::word descriptor_used_bits
    )
    {
        kernel::utility::logger::printk("lookup_capability\n");
        auto index
            = calculate_capability_index(descriptor, descriptor_used_bits);
        auto entry = index_to_capability_entry(index);
        if (entry->capability_pointer == nullptr)
        {
            kernel::utility::logger::error("null entry !\n");
        }
        return entry;
    }

    capability_entry *
        capability_node::index_to_capability_entry(common::word index)
    {
        auto index_max = 1 << radix_bits;
        if (index >= index_max)
        {
            kernel::utility::logger::error("index out of range !\n");
            return nullptr;
        }
        return &capability_slots[index];
    }
}
