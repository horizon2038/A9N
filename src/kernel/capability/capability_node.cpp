#include "kernel/capability/capability_component.hpp"
#include <kernel/capability/capability_node.hpp>
#include <kernel/utility/logger.hpp>

#include <library/capability/capability_descriptor.hpp>
#include <library/common/types.hpp>

namespace kernel
{
    capability_node::capability_node(
        common::word initial_ignore_bits,
        common::word initial_radix_bits,
        capability_component **initial_capability_slots
    )
        : capability_slots(initial_capability_slots)
        , ignore_bits(initial_ignore_bits)
        , radix_bits(initial_radix_bits)
    {
    }

    common::error capability_node::execute(message_buffer *buffer)
    {
        kernel::utility::logger::printk("execute : node\n");

        // 1. decode operation
        // 2. dispatch operation
        // 3.run operation
        return 1;
    };

    common::error capability_node::update()
    {
        return 0;
    }

    common::error capability_node::revoke()
    {
        return 0;
    };

    common::error capability_node::revoke_all()
    {
        return 0;
    }

    // recursively explores entries. this is a composite pattern that allows
    // handling single and multiple capabilities with the same interface.
    capability_component *capability_node::traverse(
        library::capability::capability_descriptor descriptor,
        common::word descriptor_max_bits, // usually WORD_BITS is used.
        common::word descriptor_used_bits
    )
    {
        auto entry = lookup_component(descriptor, descriptor_used_bits);
        if (entry == nullptr)
        {
            kernel::utility::logger::error("null entry !\n");
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
        return entry->traverse(descriptor, descriptor_max_bits, new_used_bits);
    }

    capability_component *capability_node::lookup_component(
        library::capability::capability_descriptor descriptor,
        common::word descriptor_used_bits
    )
    {
        kernel::utility::logger::printk("lookup_capability\n");
        auto index
            = calculate_capability_index(descriptor, descriptor_used_bits);
        auto component = index_to_capability_component(index);
        if (component == nullptr)
        {
            kernel::utility::logger::error("null entry !\n");
        }
        return component;
    }

    capability_component *
        capability_node::index_to_capability_component(common::word index)
    {
        auto index_max = 1 << radix_bits;
        if (index >= index_max)
        {
            kernel::utility::logger::error("index out of range !\n");
            return nullptr;
        }
        return capability_slots[index];
    }
}
