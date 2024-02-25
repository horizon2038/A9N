#ifndef GENERIC_HPP
#define GENERIC_HPP

#include <kernel/capability/capability_local_state.hpp>
#include <kernel/capability/capability_component.hpp>
#include <kernel/ipc/message_buffer.hpp>

#include <library/common/types.hpp>

namespace kernel
{
    enum class generic_data_offset : uint8_t
    {
        ADDRESS = 0x00,
        SIZE_BITS = 0x01,
        WATERMARK = 0x02,
        FLAGS = 0x03,
    };

    enum class generic_flags_shift : uint8_t
    {
        IS_DEVICE = 0x00,
    };

    enum class generic_flags_mask : common::word
    {
        IS_DEVICE = 1,
    };

    enum class generic_operation
    {
        RETYPE = 0x00
    };

    class generic final : public capability_component
    {
      public:
        common::error execute(
            capability_slot *this_slot,
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
        // these members passed from execute() argment.
        /*
        const common::physical_address start_address;
        const common::word size;
        const bool flags;
        common::physical_address watermark;
         */

        common::error decode_operation(
            message_buffer *buffer,
            capability_local_state *state
        );

        common::error
            convert(message_buffer *buffer, capability_local_state *state);

        common::error create_generic(
            message_buffer *buffer,
            capability_local_state *state
        );
    };
}

#endif
