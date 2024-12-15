#include <hal/x86_64/arch/cpu.hpp>

#include <hal/x86_64/arch/control_register.hpp>
#include <hal/x86_64/arch/segment_configurator.hpp>
#include <kernel/utility/logger.hpp>

namespace a9n::hal::x86_64
{
    hal_result init_cpu_core(void)
    {
        return init_cpu_core_features().and_then(init_cpu_core_segments);
    }

    hal_result init_cpu_core_features(void)
    {
        a9n::kernel::utility::logger::printk("configure cpu features ...\n");
        // enable wrfsbase
        _write_cr4(_read_cr4() | cr4_flag::FS_GS_BASE);
        // _write_cr0(_read_cr0() | cr0_flag::WRITE_PROTECT);

        return {};
    }

    hal_result init_cpu_core_segments(void)
    {
        return current_arch_local_variable().and_then(
            [](arch_cpu_local_variable *local_variable) -> hal_result
            {
                a9n::kernel::utility::logger::printk("configure cpu core ...\n");
                segment::configure_gdt(local_variable->gdt);
                segment::configure_idt(local_variable->idt);
                segment::configure_tss(local_variable->gdt, local_variable->tss);

                segment::load_gdt(local_variable->gdt);
                segment::load_task_register(segment_selector::KERNEL_TSS);
                segment::load_idt(local_variable->idt);

                return {};
            }
        );
    }

    liba9n::result<arch_cpu_local_variable *, hal_error> current_arch_local_variable(void)
    {
        using a9n::kernel::utility::logger;

        return a9n::hal::current_local_variable().and_then(
            [](a9n::kernel::cpu_local_variable *local_variable
            ) -> liba9n::result<arch_cpu_local_variable *, hal_error>
            {
                return &arch_cpu_local_variables[local_variable->core_number];
            }
        );
    }
}

namespace a9n::hal
{
    liba9n::result<a9n::word, hal_error> current_core_number(void)
    {
        // stub
        // TODO: return local apic address -> core id
        return 0;
    }

    hal_result configure_local_variable(a9n::kernel::cpu_local_variable *target_local_variable)
    {
        using a9n::kernel::utility::logger;

        if (!target_local_variable)
        {
            logger::printk("local variable is null!\n");
            return hal_error::ILLEGAL_ARGUMENT;
        }

        if (!target_local_variable->kernel_stack_pointer)
        {
            logger::printk("kernel stack pointer is null!\n");
            return hal_error::INIT_FIRST;
        }

        x86_64::init_cpu_core_features();

        // init kernel-level cpu_local_variable
        auto core_number = target_local_variable->core_number;
        _write_gs_base(reinterpret_cast<uint64_t>(target_local_variable));

        // init hal-level cpu_local_variable
        auto     target_arch_cpu_variable = &x86_64::arch_cpu_local_variables[core_number];
        uint64_t kernel_stack_address
            = reinterpret_cast<uint64_t>(target_local_variable->kernel_stack_pointer);
        target_arch_cpu_variable->tss.rsp_0 = kernel_stack_address;
        target_arch_cpu_variable->tss.ist_1 = kernel_stack_address;

        return {};
    }

    liba9n::result<a9n::kernel::cpu_local_variable *, hal_error> current_local_variable(void)
    {
        using a9n::kernel::utility::logger;

        auto local_variable = reinterpret_cast<a9n::kernel::cpu_local_variable *>(_read_gs_base());
        if (!local_variable)
        {
            kernel::utility::logger::printk("cpu_local_variable not found\n");
            return hal_error::NO_SUCH_ADDRESS;
        }

        return local_variable;
    }
}
