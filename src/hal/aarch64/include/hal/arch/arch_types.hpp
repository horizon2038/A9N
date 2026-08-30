#ifndef A9N_HAL_AARCH64_ARCH_TYPES_HPP
#define A9N_HAL_AARCH64_ARCH_TYPES_HPP

#include <kernel/types.hpp>

namespace a9n::hal
{
    inline constexpr a9n::word BYTE_BITS           = 8;
    inline constexpr a9n::word PAGE_SIZE           = 0x1000;
    inline constexpr a9n::word KERNEL_VIRTUAL_BASE = 0xFFFF800000000000ULL;
    inline constexpr a9n::word USER_VIRTUAL_BASE   = 0;

    // x0-x30, SP_EL0, ELR_EL1, SPSR_EL1, TPIDR_EL0.
    inline constexpr a9n::word HARDWARE_CONTEXT_SIZE = 35;
    // q0-q31 plus FPCR and FPSR (stored in word-addressable backing storage).
    inline constexpr a9n::word FLOATING_CONTEXT_SIZE    = 66;
    inline constexpr a9n::word VIRTUAL_CPU_CONTEXT_SIZE = a9n::PAGE_SIZE * 4;
    inline constexpr a9n::word VIRTUAL_CPU_STATE_COUNT  = 64;

    // GICv2 architectural IDs are wider, but A9N currently exports 256 IRQ slots.
    inline constexpr a9n::word IRQ_NUMBER_MAX              = 256;

    inline constexpr a9n::word FPU_CONTEXT_SIZE            = FLOATING_CONTEXT_SIZE;
    inline constexpr a9n::word VIRTUAL_MEMORY_CONTEXT_SIZE = 1;
    inline constexpr a9n::word INITIAL_FRAME_SIZE_BITS     = 12;
}

#endif
