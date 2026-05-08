#ifndef A9N_HAL_X86_64_ARCH_FSGSBASE_HPP
#define A9N_HAL_X86_64_ARCH_FSGSBASE_HPP

#include <hal/x86_64/arch/msr.hpp>

namespace a9n::hal::x86_64
{
    // read fs base (inline asm)
    inline a9n::virtual_address read_fs_base(void)
    {
        a9n::virtual_address fs_base;

        asm volatile("rdfsbase %0" : "=r"(fs_base) : :);

        return fs_base;
    }

    // write fs base (inline asm)
    inline void write_fs_base(a9n::virtual_address fs_base)
    {
        asm volatile("wrfsbase %0" : : "r"(fs_base) :);
    }

    // read user gs base (inline asm)
    // use rdmsr (IA32_KERNEL_GS_BASE)
    inline a9n::virtual_address read_user_gs_base(void)
    {
        // NOTE: that swapgs "swaps" the original user GS base with the kernel GS base!
        return x86_64::_read_msr(x86_64::msr::IA32_KERNEL_GS_BASE);
    }

    inline void write_user_gs_base(a9n::virtual_address user_gs_base)
    {
        // NOTE: that swapgs "swaps" the original user GS base with the kernel GS base!
        x86_64::_write_msr(x86_64::msr::IA32_KERNEL_GS_BASE, user_gs_base);
    }

    inline a9n::virtual_address read_gs_base(void)
    {
        a9n::virtual_address gs_base;

        asm volatile("rdgsbase %0" : "=r"(gs_base) : :);

        return gs_base;
    }

    inline void write_gs_base(a9n::virtual_address gs_base)
    {
        asm volatile("wrgsbase %0" : : "r"(gs_base) :);
    }
}

#endif
