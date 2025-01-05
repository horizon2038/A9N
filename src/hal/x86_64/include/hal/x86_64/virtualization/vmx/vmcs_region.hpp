#ifndef A9N_HAL_X86_64_VMCS_REGION_HPP
#define A9N_HAL_X86_64_VMCS_REGION_HPP

#include <hal/hal_result.hpp>
#include <hal/x86_64/virtualization/vmx/vmx_result.hpp>
#include <kernel/memory/memory.hpp>
#include <kernel/types.hpp>

namespace a9n::hal::x86_64
{
    // VMCS region
    // cf., Intel SDM vol. 3C 25-2, table 25-1, "Format of the VMCS Region"
    //      (combined : page 3938)
    // +----------+---------------------+-----------+-------+
    // |   byte   |          +0         |    +4     |  +8   |
    // |----------+------------+--------+-----------+-------+
    // |   bit    | 0-30       | 31     | 32-64     | 64-95 |
    // |----------+------------+--------+-----------+-------+
    // | contents | identifier | shadow | indicator | data  |
    // +----------+------------+--------+-----------+-------+

    struct vmcs_region
    {
        uint32_t field[2];
        uint32_t vmx_abort_indicator;
        uint8_t  data[a9n::PAGE_SIZE - (sizeof(uint32_t) * 3)]; // HACK: poor implementation
                                                                // (fixed-size)

        constexpr void configure_revision_id(uint32_t identifier)
        {
            identifier &= ((static_cast<uint32_t>(1) << 31) - 1);
            field[0]   |= identifier;
        }

        constexpr void configure_shadow_indicator(bool state)
        {
            field[0] |= (static_cast<uint32_t>(state) << 31);
        }

    } __attribute__((packed));

    static_assert(sizeof(vmcs_region) == a9n::PAGE_SIZE);

    // HACK: replace this
    alignas(a9n::PAGE_SIZE) inline vmcs_region vmcs_region_core;

}

#endif
