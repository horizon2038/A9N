#ifndef A9N_HAL_AARCH64_CPU_HPP
#define A9N_HAL_AARCH64_CPU_HPP

#include <hal/hal_result.hpp>
#include <kernel/process/cpu.hpp>

namespace a9n::hal::aarch64
{
    inline void data_synchronization_barrier()
    {
        asm volatile("dsb ish" ::: "memory");
    }

    inline void instruction_synchronization_barrier()
    {
        asm volatile("isb" ::: "memory");
    }

    inline void invalidate_tlb_all()
    {
        asm volatile("dsb ishst; tlbi vmalle1is; dsb ish; isb" ::: "memory");
    }
}

#endif
