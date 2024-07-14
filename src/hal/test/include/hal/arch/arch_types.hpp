#ifndef A9N_HAL_ARCH_ARCH_TYPES_HPP
#define A9N_HAL_ARCH_ARCH_TYPES_HPP

#include <kernel/types.hpp>

namespace a9n::hal
{
    inline constexpr a9n::word BYTE_BITS = 8;
    inline constexpr a9n::word PAGE_SIZE = 0x1000; // 4096

    // no virtual-memory management
    inline constexpr a9n::word KERNEL_VIRTUAL_BASE = 0x0;
    inline constexpr a9n::word USER_VIRTUAL_BASE   = 0x0;
}

#endif
