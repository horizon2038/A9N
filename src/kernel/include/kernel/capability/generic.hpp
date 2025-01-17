#ifndef GENERIC_HPP
#define GENERIC_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_local_state.hpp>
#include <kernel/ipc/ipc_buffer.hpp>
#include <kernel/memory/memory_type.hpp>

#include <kernel/utility/logger.hpp>

#include <kernel/kernel_result.hpp>
#include <kernel/types.hpp>

#include <liba9n/option/option.hpp>

namespace a9n::kernel
{
    struct memory_region
    {
        a9n::physical_address base_address;
        a9n::physical_address end_address;
    };

    struct allocate_info
    {
        a9n::physical_address aligned_base;
        a9n::physical_address new_watermark;
    };

    inline a9n::word serialize_generic_flags(bool is_device, a9n::word size_bits)
    {
        a9n::word value {};

        value               |= (static_cast<a9n::word>(is_device) << 7);
        auto size_bits_mask  = (1 << 7) - 1;
        value               |= (size_bits & size_bits_mask);

        return value;
    }

    inline a9n::word generic_flags_size_bits(a9n::word flags)
    {
        constexpr a9n::word size_mask = (1 << (7)) - 1;
        return flags & size_mask;
    }

    inline bool generic_flags_is_device(a9n::word flags)
    {
        return (flags >> 7) & 1;
    }

    class generic_info
    {
      public:
        generic_info(
            const a9n::physical_address initial_base_address,
            const a9n::word             initial_size_bits,
            const bool                  initial_device,
            const a9n::physical_address initial_watermark
        )
            : base_address { initial_base_address }
            , size_bits { initial_size_bits }
            , watermark { initial_watermark }
            , device { initial_device }
        {
        }

        a9n::physical_address base() const;

        a9n::physical_address current_watermark() const;

        bool is_device() const;

        bool is_allocatable(a9n::word memory_size_bits, a9n::word count) const;

        a9n::error apply_allocate(a9n::word memory_size_bits);

        memory_result<allocate_info> try_apply_allocate(a9n::word memory_size_bits, a9n::word count);

        inline a9n::word size() const
        {
            return static_cast<a9n::word>(1) << size_bits;
        }

        capability_slot_data dump_slot_data() const;

      private:
        const a9n::physical_address base_address;
        const a9n::word             size_bits;
        a9n::physical_address       watermark;
        const bool                  device;
    };

    class generic final : public capability_component
    {
      public:
        capability_result execute(process &this_process, capability_slot &this_slot) override;

        capability_result revoke() override;

        // empty implements (for composite-pattern)
        capability_lookup_result retrieve_slot(a9n::word index) override
        {
            return capability_lookup_error::TERMINAL;
        }

        capability_lookup_result traverse_slot(
            a9n::capability_descriptor descriptor,
            a9n::word                  max_bits,
            a9n::word                  used_bits
        ) override
        {
            return capability_lookup_error::TERMINAL;
        };

      private:
        capability_result decode_operation(process &owner, capability_slot &self);

        capability_result convert(process &owner, capability_slot &self);

        generic_info create_generic_info(const capability_slot_data &data);

        capability_lookup_result retrieve_target_root_slot(process &owner) const;

        constexpr liba9n::option<a9n::word>
            calculate_capability_memory_size_bits(capability_type type, a9n::word specific_bits);

        capability_result try_make_capability(
            capability_type  type,
            a9n::word        memory_size_bits,
            a9n::word        specific_bits,
            generic_info    &info,
            capability_slot &self,
            capability_slot &target_slot
        );
    };

    inline generic generic_core {};

    inline kernel_result try_configure_generic_slot(capability_slot &slot, const generic_info &info)
    {
        slot.component = &generic_core;
        slot.type      = capability_type::GENERIC;
        slot.data      = info.dump_slot_data();

        return {};
    }

}

#endif
