#ifndef CAPABILITY_NODE_HPP
#define CAPABILITY_NODE_HPP

#include <kernel/capability/capability.hpp>

#include <library/common/types.hpp>
#include <library/capability/capability_descriptor.hpp>

namespace kernel
{
    // users never keep the address of capability_entry.
    // capability_descriptors are only used for indirect adressing.
    struct capability_entry;

    struct capability_data
    {
        common::word data[4];
    };

    struct dependency_node
    {
        // sibling capability_entry
        capability_entry *next_capability_entry;
        capability_entry *preview_capability_entry;

        // child capability_entry
        capability_entry *child_capability_entry;
    };

    struct capability_entry
    {
        capability *capablity_pointer;
        capability_data data;
        dependency_node family_node;
    };

    struct capability_lookup_result
    {
        common::word index;
        common::word depth_bits;
    };

    class capability_node final : capability
    {
      public:
        capability_node(
            common::word initial_ignore_bits,
            common::word initial_radix_bits,
            capability_entry *initial_capability_slots
        );

        common::error execute() override;

        capability_entry traverse_capability(
            library::capability::capability_descriptor target_descriptor
        );

      private:
        common::word ignore_bits;
        common::word radix_bits;

        // the number of slots for a capability_node is not fixed,
        // and thhe number can be specified when creating it.

        // the number of slots is 2^radix_bits.
        capability_entry *capability_slots;

        capability_lookup_result lookup_capability(
            library::capability::capability_descriptor target_descriptor,
            common::word depth_bits
        );
    };

    inline const common::word calculate_capability_index(
        library::capability::capability_descriptor descriptor,
        common::word ignore_bits,
        common::word radix_bits,
        common::word depth_bits
    )
    {
        auto mask_bits = (1ull << radix_bits) - 1;
        auto shift_bits
            = (common::WORD_BITS - (ignore_bits + radix_bits + depth_bits));
        auto index = (descriptor >> shift_bits) & mask_bits;
        return index;
    }

}

#endif
