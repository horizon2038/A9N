#ifndef GENERIC_HPP
#define GENERIC_HPP

#include <kernel/capability/capability_local_state.hpp>
#include <kernel/capability/capability_component.hpp>
#include <kernel/ipc/message_buffer.hpp>

#include <library/common/types.hpp>

namespace kernel
{
    struct generic_info
    {
        common::physical_address base_address;
        common::word size_bits;
        bool is_device;
        common::physical_address watermark;
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
            capability_slot *this_slot,
            capability_slot *root_slot,
            message_buffer *buffer
        );

        common::error convert(
            capability_slot *this_slot,
            capability_slot *root_slot,
            message_buffer *buffer
        );

        common::error create_generic(
            capability_slot *this_slot,
            capability_slot *root_slot,
            message_buffer *buffer
        );

        generic_info calculate_generic_info(capability_slot_data *data);

        inline bool is_device(common::word flags)
        {
            return (flags >> 7) & 1;
        }

        inline common::word calculate_generic_size_bits(common::word flags)
        {
            common::word size_mask = (1 << (7)) - 1;
            return flags & size_mask;
        }

        capability_slot *retrieve_target_slot(
            capability_slot *root_slot,
            message_buffer *buffer
        );

        // create dest capability_slot

        generic_info create_child_generic_info(
            generic_info *parent_info,
            message_buffer *buffer
        );

        bool is_valid_size(generic_info *parent_info, generic_info *child_info);

        capability_slot_data create_generic_slot_data(generic_info *info);

        generic_info update_parent_generic_info(
            generic_info *parent_info,
            generic_info *child_info
        );

        inline library::common::word create_generic_flags(
            bool is_device,
            library::common::word size_bits
        )
        {
            library::common::word generic_flags {};

            generic_flags |= (is_device << 7);
            auto size_bits_mask = (1 << 7) - 1;
            generic_flags |= (size_bits & size_bits_mask);

            return generic_flags;
        }
    };

}

#endif
