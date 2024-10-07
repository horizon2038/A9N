#ifndef A9N_HAL_X86_64_ARCH_MSR_HPP
#define A9N_HAL_X86_64_ARCH_MSR_HPP

#include <kernel/types.hpp>

namespace a9n::hal::x86_64
{
    extern "C" void     _write_msr(uint32_t msr, uint64_t value);
    extern "C" uint64_t _read_msr(uint32_t msr);

    namespace msr
    {
        // x86_64 machine specific registers
        inline constexpr uint32_t APIC_BASE     = 0x1B;
        inline constexpr uint32_t APIC_BASE_BSP = 0x100;

        inline constexpr uint32_t EFER          = 0xC0000080;
        inline constexpr uint32_t STAR          = 0xc0000081;
        inline constexpr uint32_t LSTAR         = 0xc0000082;
        inline constexpr uint32_t CSTAR         = 0xc0000083;
        inline constexpr uint32_t SFMASK        = 0xc0000084;
    }

    namespace efer_flag
    {
        inline constexpr uint32_t SYSCALL_EXTENSION           = 1 << 0;
        inline constexpr uint32_t LONG_MODE                   = 1 << 8;
        inline constexpr uint32_t LONG_MODE_ACTIVE            = 1 << 10;
        inline constexpr uint32_t NO_EXECUTE                  = 1 << 11;
        inline constexpr uint32_t SECURE_VIRTUAL_MACHINE      = 1 << 12;
        inline constexpr uint32_t LONG_MODE_SEGMENT_LIMIT     = 1 << 13;
        inline constexpr uint32_t FAST_FX_SAVE_RESTORE        = 1 << 14;
        inline constexpr uint32_t TRANSLATION_CACHE_EXTENSION = 1 << 15;
    }
}

#endif
