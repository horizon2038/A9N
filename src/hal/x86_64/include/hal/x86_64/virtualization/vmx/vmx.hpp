#ifndef A9N_HAL_X86_64_VMX_HPP
#define A9N_HAL_X86_64_VMX_HPP

#include <kernel/memory/memory.hpp>
#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

#include <liba9n/common/enum.hpp>
#include <liba9n/libc/string.hpp>
#include <liba9n/result/result.hpp>

#include <hal/hal_result.hpp>
#include <hal/x86_64/arch/control_register.hpp>
#include <hal/x86_64/arch/cpuid.hpp>
#include <hal/x86_64/arch/msr.hpp>
#include <hal/x86_64/virtualization/vmx/vmx_result.hpp>

namespace a9n::hal::x86_64
{
    struct vmxon_region
    {
        uint32_t revision_id;
        uint8_t  data[(a9n::PAGE_SIZE - sizeof(uint32_t))];
    } __attribute__((packed));

    // vmxon_region must have page size (4KiB)
    static_assert(sizeof(vmxon_region) == a9n::PAGE_SIZE);

    namespace msr
    {
        inline constexpr uint32_t VMX_BASIC       = 0x480;

        inline constexpr uint32_t VMX_CR0_FIXED_0 = 0x486;
        inline constexpr uint32_t VMX_CR0_FIXED_1 = 0x487;
        inline constexpr uint32_t VMX_CR4_FIXED_0 = 0x488;
        inline constexpr uint32_t VMX_CR4_FIXED_1 = 0x489;
    }

    alignas(a9n::PAGE_SIZE) inline vmxon_region vmxon_region_core;

    hal_result enable_vmx(void);

}

#endif
