#include <hal/x86_64/arch/cpu.hpp>

#include <kernel/config.hpp>
#include <kernel/process/cpu.hpp>
#include <kernel/utility/logger.hpp>

#include <hal/x86_64/arch/control_register.hpp>
#include <hal/x86_64/arch/floating_point.hpp>
#include <hal/x86_64/arch/fsgsbase.hpp>
#include <hal/x86_64/arch/segment_configurator.hpp>
#include <hal/interface/lock.hpp>

namespace a9n::hal::x86_64
{
    hal_result init_cpu_core(void)
    {
        return init_cpu_core_features().and_then(init_cpu_core_segments);
    }

    hal_result init_cpu_core_features(void)
    {
        a9n::kernel::utility::logger::printh("Configuring CPU features ...\n");
        _write_cr4(
            _read_cr4()                    // :)
            | cr4_flag::FS_GS_BASE         // rdgsbase, wrgsbase, rdfsbase, wrfsbase
            | cr4_flag::PAGE_GLOBAL        // retain shared kernel mappings across CR3 reloads
            | cr4_flag::OS_X_SAVE          // xsave, xrstor, xsetbv, xgetbv
            | cr4_flag::OS_FX_SAVE_RESTORE // fxsave, fxrstor
            | cr4_flag::OS_XMM_EXCEPTION   // unmasked SIMD floating-point exceptions
        );
        configure_floating_mode();
        // _write_cr0(_read_cr0() | cr0_flag::WRITE_PROTECT);

        return { };
    }

    hal_result init_cpu_core_segments(void)
    {
        return current_arch_local_variable().and_then(
            [](arch_cpu_local_variable *local_variable) -> hal_result
            {
                a9n::kernel::utility::logger::printh("Configuring CPU core ...\n");
                segment::configure_gdt(local_variable->gdt);
                segment::configure_idt(local_variable->idt);
                segment::configure_tss(local_variable->gdt, local_variable->tss);

                segment::load_gdt(local_variable->gdt);
                // Reload CS after replacing the GDT.  This is required for an AP, which enters
                // long mode with the trampoline's code selector, and is harmless on the BSP.
                segment::load_segment_register(segment_selector::KERNEL_CS);
                segment::load_task_register(segment_selector::KERNEL_TSS);
                segment::load_idt(local_variable->idt);

                return { };
            }
        );
    }

    liba9n::result<arch_cpu_local_variable *, hal_error> current_arch_local_variable(void)
    {
        using a9n::kernel::utility::logger;

        return a9n::hal::current_local_variable().and_then(
            [](a9n::kernel::cpu_local_variable *local_variable)
                -> liba9n::result<arch_cpu_local_variable *, hal_error>
            {
                return &arch_cpu_local_variables[local_variable->core_number];
            }
        );
    }

    inline a9n::word                       current_cpu_number { 0 };
    inline a9n::kernel::spin_lock_no_owner lock;

    liba9n::result<a9n::word, hal_error> try_allocate_core_number(void)
    {
        auto lock_result = lock.lock();
        if (!lock_result)
        {
            return hal_error::TRY_AGAIN;
        }

        a9n::kernel::utility::logger::printh("Trying to allocate a core number ...\n");
        if (current_cpu_number >= a9n::kernel::CPU_COUNT_MAX - 1)
        {
            lock.unlock();
            a9n::kernel::utility::logger::error("No such CPU number!");
            return hal_error::NO_SUCH_DEVICE;
        }

        auto allocated_core_number = ++current_cpu_number;
        lock.unlock();
        return allocated_core_number;
    }

    void mark_core_booted(void)
    {
        lock.lock();
        auto count = a9n::hal::atomic_load(&booted_core_count);
        a9n::hal::atomic_store(&booted_core_count, static_cast<uint8_t>(count + 1));
        lock.unlock();
    }
}

namespace a9n::hal
{
    hal_result configure_local_variable(a9n::kernel::cpu_local_variable *target_local_variable)
    {
        using a9n::kernel::utility::logger;

        if (!target_local_variable)
        {
            logger::printh("CPU local variable is null!\n");
            return hal_error::ILLEGAL_ARGUMENT;
        }

        if (!target_local_variable->kernel_stack_pointer)
        {
            logger::printh("Kernel stack pointer is null!\n");
            return hal_error::INIT_FIRST;
        }

        x86_64::init_cpu_core_features();

        // init kernel-level cpu_local_variable
        auto core_number = target_local_variable->core_number;
        x86_64::write_gs_base(reinterpret_cast<uint64_t>(target_local_variable));

        // init hal-level cpu_local_variable
        auto     target_arch_cpu_variable = &x86_64::arch_cpu_local_variables[core_number];
        uint64_t kernel_stack_address
            = reinterpret_cast<uint64_t>(target_local_variable->kernel_stack_pointer);
        target_arch_cpu_variable->tss.rsp_0 = kernel_stack_address;
        target_arch_cpu_variable->tss.ist_1 = kernel_stack_address;

        return { };
    }

    liba9n::result<a9n::kernel::cpu_local_variable *, hal_error> current_local_variable(void)
    {
        using a9n::kernel::utility::logger;

        if (!(x86_64::_read_cr4() & x86_64::cr4_flag::FS_GS_BASE)) [[unlikely]]
        {
            return hal_error::INIT_FIRST;
        }

        auto local_variable
            = reinterpret_cast<a9n::kernel::cpu_local_variable *>(x86_64::read_gs_base());
        if (!local_variable) [[unlikely]]
        {
            kernel::utility::logger::error("CPU local variable could not be found");
            return hal_error::NO_SUCH_ADDRESS;
        }

        return local_variable;
    }

    liba9n::result<a9n::word, hal_error> current_core_number(void)
    {
        return current_local_variable().and_then(
            [](a9n::kernel::cpu_local_variable *local_variable) -> liba9n::result<a9n::word, hal_error>
            {
                return local_variable->core_number;
            }
        );
    }

    a9n::word core_count(void)
    {
        if constexpr (kernel::SMP_ENABLED)
        {
            return x86_64::expected_core_count;
        }

        return 1;
    }

    hal_result start_other_cores(void)
    {
        if constexpr (!kernel::SMP_ENABLED)
        {
            return {};
        }

        atomic_store(&x86_64::is_ap_runnable, 1);

        while (atomic_load(&x86_64::booted_core_count) < x86_64::expected_core_count)
        {
            if (atomic_load(&x86_64::has_ap_boot_failed))
            {
                return hal_error::NO_SUCH_DEVICE;
            }
            asm volatile("pause" ::: "memory");
        }

        return {};
    }

}
