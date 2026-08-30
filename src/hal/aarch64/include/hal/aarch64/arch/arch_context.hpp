#ifndef A9N_HAL_AARCH64_ARCH_CONTEXT_HPP
#define A9N_HAL_AARCH64_ARCH_CONTEXT_HPP

#include <kernel/types.hpp>

namespace a9n::hal::aarch64
{
    namespace register_index
    {
        inline constexpr a9n::word X0        = 0;
        inline constexpr a9n::word X1        = 1;
        inline constexpr a9n::word X2        = 2;
        inline constexpr a9n::word X3        = 3;
        inline constexpr a9n::word X4        = 4;
        inline constexpr a9n::word X5        = 5;
        inline constexpr a9n::word X6        = 6;
        inline constexpr a9n::word X7        = 7;
        inline constexpr a9n::word X8        = 8;
        inline constexpr a9n::word X9        = 9;
        inline constexpr a9n::word X10       = 10;
        inline constexpr a9n::word X30       = 30;
        inline constexpr a9n::word SP_EL0    = 31;
        inline constexpr a9n::word ELR_EL1   = 32;
        inline constexpr a9n::word SPSR_EL1  = 33;
        inline constexpr a9n::word TPIDR_EL0 = 34;
    }

    // SPSR_EL1.M values used when restoring a context with ERET.
    inline constexpr a9n::word SPSR_MODE_EL0T = 0x0;
    inline constexpr a9n::word SPSR_MODE_EL1H = 0x5;

    struct exception_frame
    {
        a9n::word registers[35];
        a9n::word esr_el1;
        a9n::word far_el1;
        a9n::word reserved;
    };

    static_assert(sizeof(exception_frame) == 304);

    extern "C" [[noreturn]] void aarch64_restore_context(a9n::word *context);
    extern "C" void              aarch64_save_floating_context(a9n::word *context);
    extern "C" void              aarch64_restore_floating_context(a9n::word *context);
}

#endif
