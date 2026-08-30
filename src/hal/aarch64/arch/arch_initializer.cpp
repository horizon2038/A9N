#include <hal/aarch64/arch/arch_initializer.hpp>

#include <hal/aarch64/interrupt/interrupt.hpp>
#include <hal/aarch64/platform.hpp>
#include <hal/interface/cpu.hpp>

#include <kernel/process/cpu.hpp>
#include <kernel/utility/logger.hpp>

extern "C"
{
    extern uint8_t __init_constructors_start[];
    extern uint8_t __init_constructors_end[];
}

namespace a9n::hal::aarch64
{
    namespace
    {
        void init_global_constructors()
        {
            using constructor = void (*)();
            auto *begin       = reinterpret_cast<constructor *>(__init_constructors_start);
            auto *end         = reinterpret_cast<constructor *>(__init_constructors_end);
            for (auto *current = begin; current < end; ++current)
            {
                (*current)();
            }
        }
    }

    hal_result init_architecture_impl(a9n::word arch_info[])
    {
        init_global_constructors();

        a9n::word cpacr {};
        asm volatile("mrs %0, cpacr_el1" : "=r"(cpacr));
        cpacr |= 3ULL << 20;
        asm volatile("msr cpacr_el1, %0; isb" : : "r"(cpacr) : "memory");

        asm volatile("msr vbar_el1, %0; isb" : : "r"(aarch64_exception_vectors) : "memory");

        auto local_result = a9n::hal::configure_local_variable(
            &a9n::kernel::cpu_local_variables[a9n::kernel::BSP_ID]
        );
        if (!local_result)
        {
            return local_result;
        }

        a9n::kernel::utility::logger::printh("AArch64 boot information: dtb=0x%016llx\n", arch_info[0]);

        auto interrupt_result = platform::init_interrupt_controller();
        if (!interrupt_result)
        {
            return interrupt_result;
        }

        auto timer_result = platform::init_system_timer();
        if (!timer_result)
        {
            return timer_result;
        }

        // Drop the temporary low identity map. User address spaces install TTBR0 later.
        a9n::word zero = 0;
        asm volatile("msr ttbr0_el1, %0; dsb ish; tlbi vmalle1is; dsb ish; isb"
                     :
                     : "r"(zero)
                     : "memory");

        return {};
    }
}

namespace a9n::hal
{
    hal_result init_architecture(a9n::word arch_info[])
    {
        return aarch64::init_architecture_impl(arch_info);
    }
}
