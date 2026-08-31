#ifndef A9N_HAL_AARCH64_CPU_HPP
#define A9N_HAL_AARCH64_CPU_HPP

#include <hal/hal_result.hpp>
#include <kernel/process/cpu.hpp>
#include <kernel/types.hpp>

namespace a9n::hal::aarch64
{
    enum class cpu_enable_method : uint8_t
    {
        NONE,
        PSCI,
        SPIN_TABLE,
    };

    enum class psci_conduit : uint8_t
    {
        NONE,
        HVC,
        SMC,
    };

    struct cpu_description
    {
        a9n::word             mpidr { 0 };
        a9n::physical_address release_address { 0 };
        cpu_enable_method     enable_method { cpu_enable_method::NONE };
    };

    inline cpu_description cpu_descriptions[a9n::kernel::CPU_COUNT_MAX];
    inline a9n::word       expected_core_count { 1 };
    inline psci_conduit     discovered_psci_conduit { psci_conduit::NONE };
    inline uint8_t          secondary_cores_runnable { 0 };
    inline uint8_t          booted_core_count { 1 };
    inline uint8_t          secondary_boot_failed { 0 };

    hal_result discover_cpu_topology(a9n::physical_address dtb_address);
    hal_result start_secondary_cores();
    hal_result init_current_core();
    void       mark_current_core_booted();

    extern "C" void aarch64_secondary_entry();
    extern "C" a9n::physical_address aarch64_secondary_entry_address();
    extern "C" void aarch64_ap_entry();

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
