#ifndef GENERIC_HPP
#define GENERIC_HPP

#include "kernel/capability/capability_entry.hpp"
#include <kernel/capability/capability_object.hpp>
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

    class generic final : public capability_object
    {
      public:
        generic(
            common::physical_address initial_start_address,
            common::word initial_size,
            bool initial_flags
        );

        common::error execute(
            message_buffer *buffer,
            capability_local_state *local_state
        ) override;

        common::error update() override;

        common::error revoke() override;

      private:
        const common::physical_address start_address;
        const common::word size;
        const bool flags;
        common::physical_address watermark;

        common::error convert();
    };
}

#endif
