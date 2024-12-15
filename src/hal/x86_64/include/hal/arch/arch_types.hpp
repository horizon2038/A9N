#ifndef A9N_HAL_ARCH_ARCH_TYPES_HPP
#define A9N_HAL_ARCH_ARCH_TYPES_HPP

#include <kernel/types.hpp>

namespace a9n::hal
{
    // memory
    inline constexpr a9n::word BYTE_BITS           = 8;
    inline constexpr a9n::word PAGE_SIZE           = 0x1000; // 4096
    inline constexpr a9n::word KERNEL_VIRTUAL_BASE = 0xFFFF800000000000;
    inline constexpr a9n::word USER_VIRTUAL_BASE   = 0x0;

    // context
    inline constexpr a9n::word HARDWARE_CONTEXT_SIZE = 22;

    // WIP
    inline constexpr a9n::word FPU_CONTEXT_SIZE            = 0;
    inline constexpr a9n::word VIRTUAL_MEMORY_CONTEXT_SIZE = 1;
}

#endif
