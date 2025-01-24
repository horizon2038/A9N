#ifndef A9N_HAL_X86_64_FLOATING_POINT_HPP
#define A9N_HAL_X86_64_FLOATING_POINT_HPP

#include <kernel/process/process.hpp>

namespace a9n::hal::x86_64
{
    inline static uint64_t x_save_mask = 0;

    inline namespace x_cr0_flag
    {
        inline constexpr uint64_t X87      = 1 << 0;
        inline constexpr uint64_t SSE      = 1 << 1;
        inline constexpr uint64_t AVX      = 1 << 2;
        inline constexpr uint64_t BNDREGS  = 1 << 3;
        inline constexpr uint64_t BNDCSR   = 1 << 4;
        inline constexpr uint64_t OPMASK   = 1 << 5;
        inline constexpr uint64_t ZMM_HI   = 1 << 6;
        inline constexpr uint64_t HI16_ZMM = 1 << 7;
        inline constexpr uint64_t RESERVED = 1 << 8;
        inline constexpr uint64_t PKRU     = 1 << 9;
    };

    // helper for floating context
    inline void x_save(a9n::kernel::floating_context *context, uint64_t mask)
    {
        asm volatile(
            "xsave64 %0"
            : "=m"(*context)
            : "a"(mask & 0xFFFF'FFFF),        // lower 32-bit
              "d"((mask >> 32) & 0xFFFF'FFFF) // upper 32-bit
            : "memory"
        );
    }

    inline void x_save_opt(a9n::kernel::floating_context *context, uint64_t mask)
    {
        asm volatile(
            "xsaveopt64 %0"
            : "=m"(*context)
            : "a"(mask & 0xFFFF'FFFF),        // lower 32-bit
              "d"((mask >> 32) & 0xFFFF'FFFF) // upper 32-bit
            : "memory"
        );
    }

    inline void x_restore(a9n::kernel::floating_context *context, uint64_t mask)
    {
        asm volatile(
            "xrstor64 %0"
            :
            : "m"(*context),
              "a"(mask & 0xFFFF'FFFF),        // lower 32-bit
              "d"((mask >> 32) & 0xFFFF'FFFF) // upper 32-bit
            : "memory"
        );
    }

    inline uint64_t x_get_bitmap(uint32_t index)
    {
        uint32_t low;
        uint32_t high;

        asm volatile("xgetbv" : "=a"(low), "=d"(high) : "c"(index));

        return low | (static_cast<uint64_t>(high) << 32);
    }

    inline void x_set_bitmap(uint32_t index, uint64_t value)
    {
        uint32_t low  = value & 0xFFFF'FFFF;
        uint32_t high = (value >> 32) & 0xFFFF'FFFF;

        asm volatile("xsetbv" : : "a"(low), "c"(index), "d"(high));
    }

    inline uint64_t x_get_cr0(void)
    {
        return x_get_bitmap(0);
    }

    inline uint64_t x_get_in_use(void)
    {
        return x_get_bitmap(1);
    }

    inline void configure_floating_mode(void)
    {
        uint64_t mask
            = x_get_cr0()     // :)
            | x_cr0_flag::X87 // always must be set
            | x_cr0_flag::SSE // mxcsr and xmm
            | x_cr0_flag::AVX // ymm
            ;

        x_set_bitmap(0, mask);

        x_save_mask = x_get_cr0();
    }

    inline void switch_floating_context(
        a9n::kernel::floating_context &preview,
        a9n::kernel::floating_context &next
    )
    {
        // switch floating_context
        // currently : only xsave/xrstor
        // TODO: add more operationg (e.g., fxsave/fxrstor, xsaveopt)

        x_save_opt(&preview, x_save_mask);
        x_restore(&next, x_save_mask);
    }
}

#endif
