#ifndef CAPABILITY_NODE_HPP
#define CAPABILITY_NODE_HPP

#include <kernel/capability/capability_component.hpp>

#include <kernel/types.hpp>

namespace a9n::kernel
{
    // users never keep the address of capability_entry.
    // capability_descriptors are only used for indirect adressing.
    class capability_node final : public capability_component
    {
      public:
        capability_node(
            a9n::word        initial_ignore_bits,
            a9n::word        initial_radix_bits,
            capability_slot *initial_capability_slots
        );

        capability_result execute(
            ipc_buffer      *buffer,
            capability_slot *this_slot,
            capability_slot *root_slot
        ) override;

        capability_result revoke() override
        {
            return {};
        };

        capability_lookup_result retrieve_slot(a9n::word index) override;

        capability_lookup_result traverse_slot(
            a9n::capability_descriptor descriptor,
            a9n::word                  descriptor_max_bits,
            a9n::word                  descriptor_used_bits
        ) override;

      private:
        a9n::word ignore_bits;
        a9n::word radix_bits;

        // the number of slots for a capability_node is not fixed,
        // and thhe number can be specified when creating it.
        // the number of slots is 2^radix_bits.
        capability_slot *capability_slots;

        capability_result decode_operation(
            ipc_buffer      *buffer,
            capability_slot *this_slot,
            capability_slot *root_slot
        );

        capability_result operation_copy(
            ipc_buffer      *buffer,
            capability_slot *this_slot,
            capability_slot *root_slot
        );

        capability_result operation_move(
            ipc_buffer      *buffer,
            capability_slot *this_slot,
            capability_slot *root_slot
        );

        capability_result operation_revoke(
            ipc_buffer      *buffer,
            capability_slot *this_slot,
            capability_slot *root_slot
        );

        capability_result operation_remove(
            ipc_buffer      *buffer,
            capability_slot *this_slot,
            capability_slot *root_slot
        );

        capability_lookup_result lookup_slot(
            a9n::capability_descriptor target_descriptor,
            a9n::word                  descriptor_used_bits
        );

        // inline section
        inline const bool is_index_valid(a9n::word index)
        {
            auto index_max = static_cast<a9n::word>(1 << radix_bits);

            if (index >= index_max)
            {
                return false;
            }

            return true;
        }

        inline const bool is_depth_remain(a9n::word depth)
        {
            return (depth < a9n::WORD_BITS);
        }

        inline const a9n::word calculate_capability_index(
            a9n::capability_descriptor descriptor,
            a9n::word                  descriptor_used_bits
        )
        {
            auto mask_bits = static_cast<a9n::word>((1 << radix_bits) - 1);
            auto shift_bits
                = (a9n::WORD_BITS
                   - (ignore_bits + radix_bits + descriptor_used_bits));
            auto index = (descriptor >> shift_bits) & mask_bits;
            return index;
        }

        inline const a9n::word
            calculate_used_bits(a9n::word old_descriptor_used_bits)
        {
            return (ignore_bits + radix_bits + old_descriptor_used_bits);
        }
    };
}

#endif
