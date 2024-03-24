#ifndef GENERIC_HPP
#define GENERIC_HPP

#include <kernel/capability/capability_local_state.hpp>
#include <kernel/capability/capability_component.hpp>
#include <kernel/ipc/message_buffer.hpp>

#include <library/common/types.hpp>

namespace kernel
{
    struct memory_region
    {
        common::physical_address base_address;
        common::physical_address end_address;
    };

    inline common::word
        serialize_generic_flags(bool is_device, library::common::word size_bits)
    {
        library::common::word value {};

        value |= (static_cast<library::common::word>(is_device) << 7);
        auto size_bits_mask = (1 << 7) - 1;
        value |= (size_bits & size_bits_mask);

        return value;
    }

    inline common::word generic_flags_size_bits(common::word flags)
    {
        constexpr common::word size_mask = (1 << (7)) - 1;
        return flags & size_mask;
    }

    inline bool generic_flags_is_device(common::word flags)
    {
        return (flags >> 7) & 1;
    }

    class generic_info
    {
      public:
        generic_info(
            const common::physical_address initial_base_address,
            const common::word initial_size_bits,
            const bool initial_device,
            const common::physical_address initial_watermark
        )
            : base_address { initial_base_address }
            , size_bits { initial_size_bits }
            , watermark { initial_watermark }
            , device { initial_device }
        {
        }

        common::physical_address current_watermark() const;

        bool is_device() const;

        bool is_allocatable(common::word memory_size_bits, common::word count)
            const;

        common::error apply_allocate(common::word memory_size_bits);

        capability_slot_data dump_slot_data() const;

      private:
        const common::physical_address base_address;
        const common::word size_bits;
        common::physical_address watermark;
        const bool device;

        inline common::word size() const
        {
            return static_cast<common::word>(1) << size_bits;
        }
    };

    class generic final : public capability_component
    {
      public:
        common::error execute(
            capability_slot *this_slot,
            capability_slot *root_slot,
            message_buffer *buffer
        ) override;

        common::error revoke() override;

        // empty implements (for composite-pattern)
        capability_slot *retrieve_slot(common::word index) override
        {
            return nullptr;
        }

        capability_slot *traverse_slot(
            library::capability::capability_descriptor descriptor,
            common::word max_bits,
            common::word used_bits
        ) override
        {
            return nullptr;
        };

      private:
        common::error decode_operation(
            capability_slot &this_slot,
            capability_slot &root_slot,
            const message_buffer &huffer
        );

        common::error convert(
            capability_slot &this_slot,
            capability_slot &root_slot,
            const message_buffer &buffer
        );

        generic_info create_generic_info(const capability_slot_data &data);

        capability_slot *retrieve_target_root_slot(
            const capability_slot &root_slot,
            const message_buffer &buffer
        ) const;
    };
}

#endif
