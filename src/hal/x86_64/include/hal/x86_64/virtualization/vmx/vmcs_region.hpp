#ifndef A9N_HAL_X86_64_VMCS_REGION_HPP
#define A9N_HAL_X86_64_VMCS_REGION_HPP

#include "kernel/memory/memory.hpp"
#include <hal/hal_result.hpp>
#include <hal/x86_64/virtualization/vmx/vmx_result.hpp>
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
        uint8_t  data[a9n::PAGE_SIZE]; // HACK: poor implementation (fixed-size)

        constexpr void configure_identifier(uint32_t identifier)
        {
            identifier &= ((static_cast<uint32_t>(1) << 31) - 1);
            field[0]   |= identifier;
        }

        constexpr void configure_shadow_indicator(bool state)
        {
            field[0] |= (static_cast<uint32_t>(state) << 31);
        }

    } __attribute__((packed));

    extern "C" void _vmclear(a9n::physical_address region);
    extern "C" void _vmptrld(a9n::physical_address region);

    inline vmx_result<> vm_clear(const vmcs_region &region)
    {
        auto address = a9n::kernel::virtual_to_physical_address(
            reinterpret_cast<a9n::virtual_address>(&region)
        );
        _vmclear(address);

        return check_vmx_result();
    }

    inline vmx_result<> vm_pointer_load(const vmcs_region &region)
    {
        auto address = a9n::kernel::virtual_to_physical_address(
            reinterpret_cast<a9n::virtual_address>(&region)
        );
        _vmptrld(address);

        return check_vmx_result();
    }

    inline vmx_result<> init_vmcs(vmcs_region &region)
    {
        return vm_clear(region).and_then(
            [&](void) -> vmx_result<>
            {
                return vm_pointer_load(region);
            }
        );
    }

}

#endif
