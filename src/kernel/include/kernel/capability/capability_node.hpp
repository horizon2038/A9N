#ifndef CAPABILITY_NODE_HPP
#define CAPABILITY_NODE_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_entry.hpp>

#include <library/common/types.hpp>
#include <library/capability/capability_descriptor.hpp>

namespace kernel
{
    // users never keep the address of capability_entry.
    // capability_descriptors are only used for indirect adressing.
    class capability_node final : public capability_component
    {
      public:
        capability_node(
            common::word initial_ignore_bits,
            common::word initial_radix_bits,
            capability_component **initial_capability_slots
        );

        common::error execute(message_buffer *buffer) override;

        common::error update() override;

        common::error revoke() override;

        common::error remove() override;

        // HACK (horizon2k38):
        // consider whether capability and node implementation
        // can be separated.
        capability_component *traverse(
            library::capability::capability_descriptor descriptor,
            common::word descriptor_max_bits,
            common::word descriptor_used_bits
        ) override;

      private:
        common::word ignore_bits;
        common::word radix_bits;

        // the number of slots for a capability_node is not fixed,
        // and thhe number can be specified when creating it.

        // the number of slots is 2^radix_bits.
        capability_component **capability_slots;

        capability_component *lookup_component(
            library::capability::capability_descriptor target_descriptor,
            common::word descriptor_used_bits
        );

        inline const bool is_depth_remain(common::word depth)
        {
            return (depth < library::common::WORD_BITS);
        }

        inline const common::word calculate_capability_index(
            library::capability::capability_descriptor descriptor,
            common::word descriptor_used_bits
        )
        {
            auto mask_bits = (1 << radix_bits) - 1;
            auto shift_bits
                = (common::WORD_BITS
                   - (ignore_bits + radix_bits + descriptor_used_bits));
            auto index = (descriptor >> shift_bits) & mask_bits;
            return index;
        }

        inline const common::word
            calculate_used_bits(common::word old_descriptor_used_bits)
        {
            return (ignore_bits + radix_bits + old_descriptor_used_bits);
        }

        capability_component *index_to_capability_component(common::word index);
    };

}

#endif
