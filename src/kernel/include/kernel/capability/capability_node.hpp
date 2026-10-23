#ifndef CAPABILITY_NODE_HPP
#define CAPABILITY_NODE_HPP

#include <kernel/capability/capability_component.hpp>

#include <kernel/kernel_result.hpp>
#include <kernel/memory/memory.hpp>
#include <kernel/types.hpp>

#include <liba9n/common/calculate.hpp>
#include <liba9n/common/not_null.hpp>

// temp
#include <kernel/utility/logger.hpp>

namespace a9n::kernel
{
    // users never keep the address of capability_entry.
    // capability_descriptors are only used for indirect adressing.
    class capability_node final : public capability_component
    {
      private:
        enum operation_type : a9n::word
        {
            NONE,
            COPY,
            MOVE,
            MINT,
            DEMOTE,
            REVOKE,
            REMOVE,
        };

        enum operation_index : a9n::word
        {
            RESERVED = 0,
            OPERATION_TYPE,
            DESTINATION_INDEX,
            SOURCE_DESCRIPTOR,
            NEW_RIGHTS,
        };

        a9n::word ignore_bits;
        a9n::word radix_bits;

        // the number of slots for a capability_node is not fixed,
        // and thhe number can be specified when creating it.
        // the number of slots is 2^radix_bits.
        capability_slot *capability_slots;

        capability_result operation_transfer(
            process &owner, capability_slot &self, operation_type operation
        );

        capability_result operation_demote(process &owner, capability_slot &self);

        capability_result operation_revoke(process &owner, capability_slot &self);

        capability_result operation_remove(process &owner, capability_slot &self);

        capability_lookup_result
            lookup_slot(a9n::capability_descriptor target_descriptor, a9n::word descriptor_used_bits);

        // inline section
        inline const a9n::word calculate_capability_index(
            a9n::capability_descriptor descriptor,
            a9n::word                  descriptor_used_bits
        )
        {
            auto mask_bits  = (static_cast<a9n::word>(1) << radix_bits) - 1;
            auto shift_bits = a9n::WORD_BITS - (ignore_bits + radix_bits + descriptor_used_bits);
            return (descriptor >> shift_bits) & mask_bits;
        }

        inline const a9n::word calculate_used_bits(a9n::word old_descriptor_used_bits)
        {
            return (ignore_bits + radix_bits + old_descriptor_used_bits);
        }

      public:
        // Manage dependency links and call each object's self-contained revoke.
        static kernel_result revoke_slot(capability_slot &slot);
        static kernel_result remove_slot(capability_slot &slot);

        bool is_reclaiming(void) const override;

        static constexpr bool is_geometry_valid(a9n::word ignored, a9n::word radix)
        {
            return ignored < a9n::WORD_BITS && radix != 0 && radix < a9n::WORD_BITS
                && radix <= a9n::WORD_BITS - ignored;
        }

        bool is_valid(void) const { return capability_slots != nullptr; }

        capability_node(
            a9n::word        initial_ignore_bits,
            a9n::word        initial_radix_bits,
            capability_slot *initial_capability_slots
        );

        capability_result execute(process &this_process, capability_slot &this_slot) override;

        capability_result revoke(capability_slot &self) override;

        // The caller supplies an index within this Node's radix.
        capability_lookup_result retrieve_slot(a9n::word index) override;

        capability_lookup_result traverse_slot(
            a9n::capability_descriptor descriptor,
            a9n::word                  descriptor_max_bits,
            a9n::word                  descriptor_used_bits
        ) override;
    };

    static_assert(sizeof(capability_node) == 4 * sizeof(a9n::word));

    // unsafe : no check boundary
    inline liba9n::result<liba9n::not_null<capability_node>, kernel_error>
        try_make_capability_node(a9n::virtual_address base, a9n::word radix)
    {
        if (!base || !capability_node::is_geometry_valid(0, radix))
        {
            DEBUG_LOG("illegal argument");
            return kernel_error::ILLEGAL_ARGUMENT;
        }

        constexpr auto node_unit_radix = liba9n::calculate_radix_ceil(sizeof(capability_node));
        constexpr auto node_unit_size  = static_cast<a9n::word>(1) << node_unit_radix;
        DEBUG_LOG("node_unit_radix : 0x%llx", node_unit_radix);
        DEBUG_LOG("node_unit_size : 0x%llx", node_unit_size);

        if (static_cast<a9n::word>(base) % node_unit_size != 0)
        {
            DEBUG_LOG("illegal argument");
            return kernel_error::ILLEGAL_ARGUMENT;
        }

        constexpr auto slot_unit_radix = liba9n::calculate_radix_ceil(sizeof(capability_slot));
        constexpr auto slot_unit_size  = static_cast<a9n::word>(1) << slot_unit_radix;
        if (radix >= a9n::WORD_BITS - slot_unit_radix)
        {
            return kernel_error::ILLEGAL_ARGUMENT;
        }
        DEBUG_LOG("slot_unit_radix : 0x%llx", slot_unit_radix);
        DEBUG_LOG("slot_unit_size : 0x%llx", slot_unit_size);

        // create slot
        auto node_end_address   = base + node_unit_size;
        auto slot_start_address = reinterpret_cast<a9n::physical_address>(
            liba9n::align_value(node_end_address, slot_unit_size)
        );
        DEBUG_LOG("node_end_address : 0x%llx", node_end_address);
        DEBUG_LOG("slot_start_address : 0x%llx", slot_start_address);
        auto created_slots = new (reinterpret_cast<void *>(slot_start_address))
            capability_slot[static_cast<a9n::word>(1) << radix] {};

        // create node
        DEBUG_LOG("create node");
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
        if (!node.is_valid())
        {
            return kernel_error::ILLEGAL_ARGUMENT;
        }

        slot.init();
        slot.component = &node;
        slot.type      = capability_type::NODE;
        slot.rights    = capability_slot::object_rights::ALL;
        slot.data.fill(0);

        return {};
    }
}
#endif
