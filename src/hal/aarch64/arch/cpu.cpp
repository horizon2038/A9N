#include <hal/interface/cpu.hpp>
#include <hal/interface/interrupt.hpp>

#include <kernel/process/cpu.hpp>

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
        return 1;
    }

    hal_result start_other_cores()
    {
        // PSCI-based SMP startup is intentionally left out of the first QEMU platform port.
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
