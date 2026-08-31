#include <hal/aarch64/arch/cpu.hpp>
#include <hal/aarch64/interrupt/interrupt.hpp>
#include <hal/aarch64/platform.hpp>
#include <hal/interface/cpu.hpp>
#include <hal/interface/process_manager.hpp>

#include <kernel/config.hpp>
#include <kernel/process/cpu.hpp>
#include <kernel/process/lock.hpp>
#include <kernel/time/timer.hpp>
#include <kernel/utility/logger.hpp>

namespace a9n::hal::aarch64
{
    namespace
    {
        constexpr a9n::word MPIDR_AFFINITY_MASK = 0xff00ffffffULL;

        liba9n::result<a9n::word, hal_error> current_logical_core()
        {
            a9n::word mpidr {};
            asm volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
            const auto affinity = mpidr & MPIDR_AFFINITY_MASK;
            for (a9n::word index = 0; index < expected_core_count; ++index)
            {
                if ((cpu_descriptions[index].mpidr & MPIDR_AFFINITY_MASK) == affinity)
                {
                    return index;
                }
            }
            return hal_error::NO_SUCH_DEVICE;
        }

        [[noreturn]] void secondary_halt()
        {
            asm volatile("msr daifset, #0xf" ::: "memory");
            for (;;)
            {
                asm volatile("wfe");
            }
        }

        void mark_secondary_boot_failed()
        {
            a9n::hal::atomic_store(&secondary_boot_failed, 1);
            asm volatile("sev" ::: "memory");
        }
    }

    hal_result init_current_core()
    {
        a9n::word mpidr {};
        a9n::word current_el {};
        asm volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
        asm volatile("mrs %0, CurrentEL" : "=r"(current_el));

        a9n::word cpacr {};
        asm volatile("mrs %0, cpacr_el1" : "=r"(cpacr));
        cpacr |= 3ULL << 20;
        asm volatile("msr cpacr_el1, %0; isb" : : "r"(cpacr) : "memory");
        asm volatile("msr vbar_el1, %0; isb" : : "r"(aarch64_exception_vectors) : "memory");
        a9n::kernel::utility::logger::printh(
            "AArch64 core state: MPIDR=0x%016llx, EL=%llu, VBAR_EL1=0x%016llx\n",
            mpidr,
            current_el >> 2,
            reinterpret_cast<a9n::word>(aarch64_exception_vectors)
        );
        return {};
    }

    hal_result start_secondary_cores()
    {
        if constexpr (!a9n::kernel::SMP_ENABLED)
        {
            return {};
        }

        const auto entry = aarch64_secondary_entry_address();
        if (expected_core_count == 1)
        {
            a9n::kernel::utility::logger::printh(
                "No AArch64 secondary CPUs were reported by the DTB.\n"
            );
            return {};
        }

        a9n::kernel::utility::logger::printh(
            "AArch64 secondary entry: physical-address=0x%016llx, targets=%llu\n",
            entry,
            expected_core_count - 1
        );
        for (a9n::word core = 1; core < expected_core_count; ++core)
        {
            a9n::kernel::utility::logger::printh(
                "Starting AArch64 core %llu (MPIDR=0x%016llx) ...\n",
                core,
                cpu_descriptions[core].mpidr
            );
            auto result = platform::start_cpu(cpu_descriptions[core], entry);
            if (!result)
            {
                a9n::kernel::utility::logger::printh(
                    "AArch64 core %llu startup request failed: %s\n",
                    core,
                    hal_error_to_string(result.unwrap_error())
                );
                return result;
            }
            a9n::kernel::utility::logger::printh(
                "AArch64 core %llu startup request accepted.\n",
                core
            );
        }
        return {};
    }

    void mark_current_core_booted()
    {
        __atomic_add_fetch(&booted_core_count, static_cast<uint8_t>(1), __ATOMIC_ACQ_REL);
        asm volatile("sev" ::: "memory");
    }

    extern "C" void aarch64_ap_entry()
    {
        auto core_result = current_logical_core();
        if (!core_result)
        {
            a9n::word mpidr {};
            asm volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
            a9n::kernel::utility::logger::printh(
                "AArch64 AP with MPIDR=0x%016llx is absent from the DTB topology.\n",
                mpidr
            );
            mark_secondary_boot_failed();
            secondary_halt();
        }
        const auto core = core_result.unwrap();

        auto result
            = a9n::hal::configure_local_variable(&a9n::kernel::cpu_local_variables[core]);
        if (!result)
        {
            a9n::kernel::utility::logger::printh(
                "AArch64 core %llu failed to configure TPIDR_EL1: %s\n",
                core,
                hal_error_to_string(result.unwrap_error())
            );
            mark_secondary_boot_failed();
            secondary_halt();
        }

        a9n::kernel::utility::logger::printh("AArch64 core %llu entered the AP path.\n", core);

        result = init_current_core();
        if (!result)
        {
            a9n::kernel::utility::logger::printh(
                "AArch64 core %llu failed to initialize architectural state: %s\n",
                core,
                hal_error_to_string(result.unwrap_error())
            );
            mark_secondary_boot_failed();
            secondary_halt();
        }

        a9n::kernel::utility::logger::printh(
            "AArch64 core %llu is initializing its GICv2 CPU interface ...\n",
            core
        );
        result = platform::init_current_core_interrupt_controller();
        if (!result)
        {
            a9n::kernel::utility::logger::printh(
                "AArch64 core %llu failed to initialize GICv2: %s\n",
                core,
                hal_error_to_string(result.unwrap_error())
            );
            mark_secondary_boot_failed();
            secondary_halt();
        }

        a9n::kernel::utility::logger::printh(
            "AArch64 core %llu is initializing its generic timer PPI ...\n",
            core
        );
        result = platform::init_system_timer();
        if (!result)
        {
            a9n::kernel::utility::logger::printh(
                "AArch64 core %llu failed to initialize its generic timer PPI: %s\n",
                core,
                hal_error_to_string(result.unwrap_error())
            );
            mark_secondary_boot_failed();
            secondary_halt();
        }

        a9n::kernel::utility::logger::printh(
            "AArch64 core %llu is waiting for the BSP release barrier.\n",
            core
        );
        while (!a9n::hal::atomic_load(&secondary_cores_runnable))
        {
            asm volatile("wfe" ::: "memory");
        }

        a9n::kernel::utility::logger::printh(
            "AArch64 core %llu was released; initializing its process manager ...\n",
            core
        );
        auto kernel_result
            = a9n::kernel::cpu_local_variables[core].process_manager_core.init(core);
        if (!kernel_result)
        {
            a9n::kernel::utility::logger::printh(
                "AArch64 core %llu failed to initialize its process manager.\n",
                core
            );
            mark_secondary_boot_failed();
            secondary_halt();
        }

        a9n::kernel::utility::logger::printh(
            "AArch64 core %llu is starting its generic timer ...\n",
            core
        );
        result = platform::start_system_timer();
        if (!result)
        {
            a9n::kernel::utility::logger::printh(
                "AArch64 core %llu failed to start its generic timer: %s\n",
                core,
                hal_error_to_string(result.unwrap_error())
            );
            mark_secondary_boot_failed();
            secondary_halt();
        }

        a9n::kernel::utility::logger::printh(
            "AArch64 core %llu is switching to its IDLE process ...\n",
            core
        );
        {
            a9n::kernel::lock_guard guard(a9n::kernel::giant_lock);
            kernel_result
                = a9n::kernel::cpu_local_variables[core].process_manager_core.switch_to_idle();
        }
        if (!kernel_result)
        {
            a9n::kernel::utility::logger::printh(
                "AArch64 core %llu failed to switch to IDLE.\n",
                core
            );
            mark_secondary_boot_failed();
            secondary_halt();
        }

        a9n::kernel::utility::logger::printh("AArch64 core %llu is online.\n", core);
        mark_current_core_booted();
        auto restore_result = a9n::hal::restore_context(a9n::hal::cpu_mode::KERNEL);
        if (!restore_result)
        {
            a9n::kernel::utility::logger::printh(
                "AArch64 core %llu failed to restore its IDLE context: %s\n",
                core,
                hal_error_to_string(restore_result.unwrap_error())
            );
            mark_secondary_boot_failed();
        }
        secondary_halt();
    }
}

namespace a9n::hal
{
    hal_result configure_local_variable(a9n::kernel::cpu_local_variable *target_local_variable)
    {
        if (!target_local_variable)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }
        if (!target_local_variable->kernel_stack_pointer)
        {
            return hal_error::INIT_FIRST;
        }

        asm volatile("msr tpidr_el1, %0; isb" : : "r"(target_local_variable) : "memory");
        return {};
    }

    liba9n::result<a9n::kernel::cpu_local_variable *, hal_error> current_local_variable()
    {
        a9n::kernel::cpu_local_variable *local_variable {};
        asm volatile("mrs %0, tpidr_el1" : "=r"(local_variable));
        if (!local_variable)
        {
            return hal_error::INIT_FIRST;
        }
        return local_variable;
    }

    liba9n::result<a9n::word, hal_error> current_core_number()
    {
        return current_local_variable().transform(
            [](a9n::kernel::cpu_local_variable *local_variable) -> a9n::word
            {
                return local_variable->core_number;
            }
        );
    }

    a9n::word core_count()
    {
        if constexpr (a9n::kernel::SMP_ENABLED)
        {
            return aarch64::expected_core_count;
        }
        return 1;
    }

    hal_result start_other_cores()
    {
        if constexpr (!a9n::kernel::SMP_ENABLED)
        {
            return {};
        }

        if (aarch64::expected_core_count == 1)
        {
            a9n::kernel::utility::logger::printh("AArch64 BSP is the only online CPU.\n");
            return {};
        }

        a9n::kernel::utility::logger::printh(
            "Releasing %llu AArch64 secondary CPUs ...\n",
            aarch64::expected_core_count - 1
        );
        atomic_store(&aarch64::secondary_cores_runnable, 1);
        asm volatile("sev" ::: "memory");
        a9n::word frequency {};
        a9n::word started_at {};
        asm volatile("mrs %0, cntfrq_el0" : "=r"(frequency));
        asm volatile("mrs %0, cntpct_el0" : "=r"(started_at));
        while (atomic_load(&aarch64::booted_core_count) < aarch64::expected_core_count)
        {
            if (atomic_load(&aarch64::secondary_boot_failed))
            {
                a9n::kernel::utility::logger::printh(
                    "AArch64 secondary CPU initialization failed (%llu/%llu online).\n",
                    static_cast<a9n::word>(atomic_load(&aarch64::booted_core_count)),
                    aarch64::expected_core_count
                );
                return hal_error::NO_SUCH_DEVICE;
            }
            a9n::word now {};
            asm volatile("mrs %0, cntpct_el0" : "=r"(now));
            if (!frequency || now - started_at > frequency * 5)
            {
                a9n::kernel::utility::logger::printh(
                    "Timed out waiting for AArch64 secondary CPUs (%llu/%llu online).\n",
                    static_cast<a9n::word>(atomic_load(&aarch64::booted_core_count)),
                    aarch64::expected_core_count
                );
                return hal_error::TIMEOUT;
            }
            asm volatile("yield" ::: "memory");
        }
        a9n::kernel::utility::logger::printh(
            "All AArch64 CPUs are online (%llu/%llu).\n",
            static_cast<a9n::word>(atomic_load(&aarch64::booted_core_count)),
            aarch64::expected_core_count
        );
        return {};
    }

    void lock()
    {
        asm volatile("msr daifset, #2" ::: "memory");
    }

    void unlock()
    {
        asm volatile("msr daifclr, #2" ::: "memory");
    }
}
