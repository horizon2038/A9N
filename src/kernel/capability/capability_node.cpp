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

    capability_data capability_node::lookup_capability_data(
        library::capability::capability_descriptor descriptor
    )
    {
        auto mask_bits = (1ull << radix_bits) - 1;
        auto shift_bits = (common::WORD_BITS - (ignore_bits + radix_bits));
        auto index = (descriptor >> shift_bits) & mask_bits;
        auto capability_entry = capability_slots[index];
        return capability_entry.capability;
    }
}
