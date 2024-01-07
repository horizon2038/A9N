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

    common::error capability_node::capability_node::execute()
    {
        return 1;
    };

    capability_entry traverse_capability(
        library::capability::capability_descriptor descriptor
    )
    {
    }

    capability_lookup_result capability_node::lookup_capability(
        library::capability::capability_descriptor descriptor,
        common::word depth_bits
    )
    {
        /*
        auto mask_bits = (1ull << radix_bits) - 1;
        auto shift_bits
            = (common::WORD_BITS - (ignore_bits + radix_bits + depth_bits));
        auto index = (descriptor >> shift_bits) & mask_bits;
        */
        auto index = calculate_capability_index(
            descriptor,
            ignore_bits,
            radix_bits,
            depth_bits
        );
        auto depth = (ignore_bits + radix_bits);
        auto result
            = capability_lookup_result { .index = index, .depth_bits = 0 };
        return result;
    }
}
