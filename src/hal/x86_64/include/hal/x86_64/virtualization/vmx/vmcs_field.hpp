#ifndef A9N_HAL_X86_64_VMCS_FIELD_HPP
#define A9N_HAL_X86_64_VMCS_FIELD_HPP

#include <kernel/types.hpp>
#include <liba9n/common/enum.hpp>

namespace a9n::hal::x86_64
{
    // VMCS field encoding
    // cf., Intel SDM vol. 3C 25-30, table 25-21, "Structure of VMCS Component Encoding"
    //      (combined : page 3966)
    // +-------------+-------+-------+----+-------+-------+
    // |      0      |  1-9  | 10-11 | 12 | 13-14 | 15-31 |
    // +-------------+-------+-------+----+-------+-------+
    // | access type | index | type  |  - | width |     - |
    // +-------------+-------+-------+----+-------+-------+

    enum class vmcs_access_type : bool
    {
        FULL = false,
        HIGH = true,
    };

    enum class vmcs_type : uint8_t
    {
        CONTROL             = 0,
        VM_EXIT_INFORMATION = 1,
        GUEST_STATE         = 2,
        HOST_STATE          = 3
    };

    enum class vmcs_width : uint8_t
    {
        BIT_16        = 0,
        BIT_64        = 1,
        BIT_32        = 1,
        NATURAL_WIDTH = 3,
    };

    struct vmcs_field
    {
        uint32_t all;

        constexpr vmcs_field(vmcs_access_type access_type, uint16_t index, vmcs_type type, vmcs_width width)
        {
            configure_access_type(access_type);
            configure_index(index);
            configure_type(type);
            configure_width(width);
            clear_reserved_section();
        }

        constexpr void configure_access_type(vmcs_access_type access_type)
        {
            all |= liba9n::enum_cast(access_type);
        }

        constexpr void configure_index(uint16_t index)
        {
            index &= ((1 << 8) - 1);
            all   |= index << 1;
        }

        constexpr void configure_type(vmcs_type type)
        {
            all |= (static_cast<uint32_t>(liba9n::enum_cast(type)) << 10);
        }

        constexpr void configure_width(vmcs_width width)
        {
            all |= (static_cast<uint32_t>(liba9n::enum_cast(width)) << 13);
        }

        constexpr void clear_reserved_section(void)
        {
            // zero
            uint32_t reserved_1  = (static_cast<uint32_t>(1) << 12);
            uint32_t reserved_2  = ((static_cast<uint32_t>(1) << 17) - 1) << 15;

            all                 &= ~(reserved_1 | reserved_2);
        }
    };
}

#endif
