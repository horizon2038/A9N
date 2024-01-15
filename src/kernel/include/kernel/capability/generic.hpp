#ifndef GENERIC_HPP
#define GENERIC_HPP

#include <kernel/capability/capability.hpp>

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
        capability_type type() override;
        common::error execute(capability_data data) override;
        capability_entry *traverse_entry(
            library::capability::capability_descriptor descriptor,
            common::word depth
        ) override;

      private:
    };
}

#endif
