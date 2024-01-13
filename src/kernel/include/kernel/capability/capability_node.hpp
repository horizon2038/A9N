#ifndef CAPABILITY_NODE_HPP
#define CAPABILITY_NODE_HPP

#include <kernel/capability/capability.hpp>

#include <library/common/types.hpp>
#include <library/capability/capability_descriptor.hpp>

namespace kernel
{
    // users never keep the address of capability_entry.
    // capability_descriptors are only used for indirect adressing.

    struct capability_lookup_result
    {
        capability_entry *entry;
        common::word depth_bits;
    };

    class capability_node final : public capability
    {
      public:
        capability_node(
            common::word initial_ignore_bits,
            common::word initial_radix_bits,
            capability_entry *initial_capability_slots
        );

        capability_type type() override;
        common::error execute(capability_data data) override;

        capability_entry *lookup_entry(
            library::capability::capability_descriptor descriptor,
            common::word depth
        ) override;

        capability_entry *traverse_capability(
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

        inline const common::word calculate_capability_index(
            library::capability::capability_descriptor descriptor,
            common::word depth_bits
        )
        {
            auto mask_bits = (1 << radix_bits) - 1;
            auto shift_bits
                = (common::WORD_BITS - (ignore_bits + radix_bits + depth_bits));
            auto index = (descriptor >> shift_bits) & mask_bits;
            return index;
        }

        capability_entry *index_to_capability_entry(common::word index);
    };

}

#endif
