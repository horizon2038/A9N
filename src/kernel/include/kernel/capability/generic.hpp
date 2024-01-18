#ifndef GENERIC_HPP
#define GENERIC_HPP

#include <kernel/ipc/message_buffer.hpp>
#include <kernel/capability/capability.hpp>

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

    class generic final : public capability
    {
      public:
        generic(
            common::physical_address initial_start_address,
            common::word initial_size,
            bool initial_flags
        );

        common::error execute(
            message_buffer *buffer,
            capability_entry *stored_entry
        ) override;

        common::error revoke() override;

        capability_type type() override;

        capability_entry *traverse_entry(
            library::capability::capability_descriptor descriptor,
            common::word descriptor_max_bits,
            common::word descriptor_used_bits
        ) override;

      private:
        const common::physical_address start_address;
        const common::word size;
        const bool flags;
        common::physical_address watermark;
    };
}

#endif
