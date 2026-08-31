#include <hal/aarch64/arch/arch_initializer.hpp>

#include <hal/aarch64/arch/cpu.hpp>
#include <hal/aarch64/interrupt/interrupt.hpp>
#include <hal/aarch64/platform.hpp>
#include <hal/interface/cpu.hpp>

#include <kernel/config.hpp>
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
        a9n::kernel::utility::logger::printh("Initializing AArch64 HAL ...\n");
        a9n::kernel::utility::logger::printh(
            "AArch64 platform: %s\n",
            platform::PLATFORM_NAME
        );

        a9n::kernel::utility::logger::printh("Configuring BSP CPU-local state ...\n");
        auto local_result = a9n::hal::configure_local_variable(
            &a9n::kernel::cpu_local_variables[a9n::kernel::BSP_ID]
        );
        if (!local_result)
        {
            a9n::kernel::utility::logger::printh(
                "Failed to configure BSP CPU-local state: %s\n",
                hal_error_to_string(local_result.unwrap_error())
            );
            return local_result;
        }
        auto core_result = init_current_core();
        if (!core_result)
        {
            a9n::kernel::utility::logger::printh(
                "Failed to initialize BSP architectural state: %s\n",
                hal_error_to_string(core_result.unwrap_error())
            );
            return core_result;
        }

        a9n::kernel::utility::logger::printh("AArch64 boot information: dtb=0x%016llx\n", arch_info[0]);

        if constexpr (a9n::kernel::SMP_ENABLED)
        {
            a9n::kernel::utility::logger::printh("Discovering AArch64 CPUs from the DTB ...\n");
            auto topology_result = discover_cpu_topology(arch_info[0]);
            if (!topology_result)
            {
                a9n::kernel::utility::logger::printh(
                    "Failed to discover AArch64 CPU topology: %s\n",
                    hal_error_to_string(topology_result.unwrap_error())
                );
                return topology_result;
            }
        }
        else
        {
            a9n::kernel::utility::logger::printh("AArch64 SMP is disabled; using the BSP only.\n");
        }

        a9n::kernel::utility::logger::printh("Initializing GICv2 interrupt controller ...\n");
        auto interrupt_result = platform::init_interrupt_controller();
        if (!interrupt_result)
        {
            a9n::kernel::utility::logger::printh(
                "Failed to initialize GICv2: %s\n",
                hal_error_to_string(interrupt_result.unwrap_error())
            );
            return interrupt_result;
        }

        a9n::kernel::utility::logger::printh("Initializing AArch64 generic timer ...\n");
        auto timer_result = platform::init_system_timer();
        if (!timer_result)
        {
            a9n::kernel::utility::logger::printh(
                "Failed to initialize AArch64 generic timer: %s\n",
                hal_error_to_string(timer_result.unwrap_error())
            );
            return timer_result;
        }

        if constexpr (a9n::kernel::SMP_ENABLED)
        {
            a9n::kernel::utility::logger::printh("Submitting secondary CPU startup requests ...\n");
            auto secondary_result = start_secondary_cores();
            if (!secondary_result)
            {
                a9n::kernel::utility::logger::printh(
                    "Failed to start AArch64 secondary CPUs: %s\n",
                    hal_error_to_string(secondary_result.unwrap_error())
                );
                return secondary_result;
            }
        }

        // Drop the temporary low identity map. User address spaces install TTBR0 later.
        a9n::kernel::utility::logger::printh("Removing the temporary TTBR0 identity mapping ...\n");
        a9n::word zero = 0;
        asm volatile("msr ttbr0_el1, %0; dsb ish; tlbi vmalle1is; dsb ish; isb"
                     :
                     : "r"(zero)
                     : "memory");

        a9n::kernel::utility::logger::printh("AArch64 BSP HAL initialization completed.\n");
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
