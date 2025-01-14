#ifndef CAPABILITY_NODE_HPP
#define CAPABILITY_NODE_HPP

#include <kernel/capability/capability_component.hpp>

#include <kernel/kernel_result.hpp>
#include <kernel/types.hpp>

#include <liba9n/common/calculate.hpp>
#include <liba9n/common/not_null.hpp>

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

        capability_result execute(process &this_process, capability_slot &this_slot) override;

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

        capability_result decode_operation(process &this_process, capability_slot &this_slot);

        capability_result operation_copy(process &this_process, capability_slot &this_slot);

        capability_result operation_move(process &this_proecess, capability_slot &this_slot);

        capability_result operation_revoke(process &this_process, capability_slot &this_slot);

        capability_result operation_remove(process &this_process, capability_slot &this_slot);

        capability_lookup_result
            lookup_slot(a9n::capability_descriptor target_descriptor, a9n::word descriptor_used_bits);

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
            auto mask_bits  = static_cast<a9n::word>((1 << radix_bits) - 1);
            auto shift_bits = (a9n::WORD_BITS - (ignore_bits + radix_bits + descriptor_used_bits));
            auto index      = (descriptor >> shift_bits) & mask_bits;
            return index;
        }

        inline const a9n::word calculate_used_bits(a9n::word old_descriptor_used_bits)
        {
            return (ignore_bits + radix_bits + old_descriptor_used_bits);
        }
    };

    // unsafe : no check boundary
    inline liba9n::result<liba9n::not_null<capability_node>, kernel_error>
        try_make_capability_node(a9n::virtual_pointer base, a9n::word radix)
    {
        if (!base || radix == 0 || radix >= a9n::WORD_BITS)
        {
            return kernel_error::ILLEGAL_ARGUMENT;
        }

        constexpr auto node_unit_radix = liba9n::calculate_radix_ceil(sizeof(capability_node));
        constexpr auto node_unit_size  = static_cast<a9n::word>(1) << node_unit_radix;

        if (static_cast<a9n::word>(base) % node_unit_size != 0)
        {
            return kernel_error::ILLEGAL_ARGUMENT;
        }

        constexpr auto slot_unit_radix = liba9n::calculate_radix_ceil(sizeof(capability_slot));
        constexpr auto slot_unit_size  = static_cast<a9n::word>(1) << slot_unit_radix;

        // create slot
        auto slot_start_address
            = reinterpret_cast<void *>(liba9n::align_value((base + node_unit_radix), node_unit_size));
        auto created_slots = new (slot_start_address)
            capability_slot[static_cast<a9n::word>(1) << radix] {};

        // create node
        auto created_node = new (reinterpret_cast<void *>(base))
            capability_node(0, radix, created_slots);

        return liba9n::not_null<capability_node>(*created_node);
    }

    inline kernel_result
        try_configure_capability_node_slot(capability_slot &slot, capability_node &node)
    {
        if (slot.type != capability_type::NONE)
        {
            return kernel_error::INIT_FIRST;
        }

        slot.component = &node;
        slot.type      = capability_type::NODE;
        slot.data.fill(0);

        return {};
    }
}
#endif
