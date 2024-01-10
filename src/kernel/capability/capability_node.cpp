#include "library/capability/capability_descriptor.hpp"
#include "library/common/types.hpp"
#include <kernel/capability/capability_node.hpp>

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

    capability_type capability_node::type()
    {
        return capability_type::NODE;
    }

    common::error capability_node::capability_node::execute(capability_data data
    )
    {
        // 1. decode operation
        // 2. dispatch operation
        // 3.run operation
        return 1;
    };

    // traverse nodes across multiple depths.
    capability_entry *capability_node::traverse_capability(
        library::capability::capability_descriptor descriptor
    )
    {
        auto depth = 0;
        capability *target_capability = this;
        capability_entry *entry;

        while (1)
        {
            auto is_node = target_capability->type() == capability_type::NODE;
            auto is_depth_remain = ((depth < library::common::WORD_BITS));

            if (!is_depth_remain)
            {
                return entry;
            }

            if (!is_node)
            {
                // invalid depth
                return nullptr;
            }

            auto node = static_cast<capability_node *>(target_capability);
            auto lookup_result = node->lookup_capability(descriptor, depth);

            entry = lookup_result.entry;
            depth = lookup_result.depth_bits;

            if (depth > library::common::WORD_BITS)
            {
                return nullptr;
            }
            target_capability = lookup_result.entry->capablity_pointer;
        }

        // unreachable
        return nullptr;
    }

    capability_lookup_result capability_node::lookup_capability(
        library::capability::capability_descriptor descriptor,
        common::word depth_bits
    )
    {
        auto index = calculate_capability_index(descriptor, depth_bits);
        auto entry = index_to_capability_entry(index);
        auto depth = (ignore_bits + radix_bits + depth_bits);
        auto result
            = capability_lookup_result { .entry = entry, .depth_bits = depth };
        return result;
    }

    capability_entry *
        capability_node::index_to_capability_entry(common::word index)
    {
        auto index_max = 1 << radix_bits;
        if (index >= index_max)
        {
            return nullptr;
        }
        return &capability_slots[index];
    }
}
