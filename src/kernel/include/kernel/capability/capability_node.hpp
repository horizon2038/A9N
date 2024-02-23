#ifndef CAPABILITY_NODE_HPP
#define CAPABILITY_NODE_HPP

#include "kernel/ipc/message_buffer.hpp"
#include <kernel/capability/capability_component.hpp>

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
            capability_slot *initial_capability_slots
        );

        common::error execute(message_buffer *buffer) override;

        common::error add_child(
            common::word index,
            capability_component *component
        ) override;

        capability_slot *retrieve_slot(common::word index) override;

        common::error revoke_child(common::word index) override;

        common::error remove_child(common::word index) override;

        common::error revoke() override;

        common::error remove() override;

        capability_slot *traverse_slot(
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
        capability_slot *capability_slots;

        common::error decode_operation(message_buffer *buffer);

        common::error operation_copy(message_buffer *buffer);

        bool is_index_valid(common::word index) const;

        common::error operation_move(message_buffer *buffer);

        common::error operation_revoke(message_buffer *buffer);

        common::error operation_remove(message_buffer *buffer);

        capability_slot *lookup_slot(
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
            auto mask_bits = static_cast<common::word>((1 << radix_bits) - 1);
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

        capability_slot *index_to_capability_slot(common::word index);
    };

}

#endif
