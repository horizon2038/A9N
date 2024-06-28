#ifndef GENERIC_HPP
#define GENERIC_HPP

#include <kernel/capability/capability_local_state.hpp>
#include <kernel/capability/capability_component.hpp>
#include <kernel/ipc/message_buffer.hpp>

#include <liba9n/common/types.hpp>

namespace a9n::kernel
{
    struct memory_region
    {
        a9n::physical_address base_address;
        a9n::physical_address end_address;
    };

    inline a9n::word
        serialize_generic_flags(bool is_device, a9n::word size_bits)
    {
        a9n::word value {};

        value |= (static_cast<a9n::word>(is_device) << 7);
        auto size_bits_mask = (1 << 7) - 1;
        value |= (size_bits & size_bits_mask);

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

        a9n::physical_address current_watermark() const;

        bool is_device() const;

        bool is_allocatable(a9n::word memory_size_bits, a9n::word count) const;

        a9n::error apply_allocate(a9n::word memory_size_bits);

        capability_slot_data dump_slot_data() const;

      private:
        const a9n::physical_address base_address;
        const a9n::word             size_bits;
        a9n::physical_address       watermark;
        const bool                  device;

        inline a9n::word size() const
        {
            return static_cast<a9n::word>(1) << size_bits;
        }
    };

    class generic final : public capability_component
    {
      public:
        a9n::error execute(
            capability_slot *this_slot,
            capability_slot *root_slot,
            message_buffer  *buffer
        ) override;

        a9n::error revoke() override;

        // empty implements (for composite-pattern)
        capability_slot *retrieve_slot(a9n::word index) override
        {
            return nullptr;
        }

        capability_slot *traverse_slot(
            liba9n::capability::capability_descriptor descriptor,
            a9n::word                                 max_bits,
            a9n::word                                 used_bits
        ) override
        {
            return nullptr;
        };

      private:
        a9n::error decode_operation(
            capability_slot      &this_slot,
            capability_slot      &root_slot,
            const message_buffer &huffer
        );

        a9n::error convert(
            capability_slot      &this_slot,
            capability_slot      &root_slot,
            const message_buffer &buffer
        );

        generic_info create_generic_info(const capability_slot_data &data);

        capability_slot *retrieve_target_root_slot(
            const capability_slot &root_slot,
            const message_buffer  &buffer
        ) const;
    };
}

#endif
