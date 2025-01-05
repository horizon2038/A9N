#ifndef A9N_HAL_X86_64_VMX_MSR_HPP
#define A9N_HAL_X86_64_VMX_MSR_HPP

#include <kernel/types.hpp>

namespace a9n::hal::x86_64
{

    namespace msr
    {
        inline constexpr uint32_t VMX_BASIC       = 0x480;

        inline constexpr uint32_t VMX_CR0_FIXED_0 = 0x486;
        inline constexpr uint32_t VMX_CR0_FIXED_1 = 0x487;
        inline constexpr uint32_t VMX_CR4_FIXED_0 = 0x488;
        inline constexpr uint32_t VMX_CR4_FIXED_1 = 0x489;
    }
}

#endif
