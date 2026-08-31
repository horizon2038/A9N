#include <hal/aarch64/platform.hpp>

#include <hal/arch/arch_types.hpp>
#include <kernel/memory/memory.hpp>
#include <kernel/utility/logger.hpp>

namespace a9n::hal::aarch64::platform
{
    namespace
    {
        inline a9n::word interrupt_count = 0;

        volatile uint32_t &gicd(a9n::word offset)
        {
            return *a9n::kernel::physical_to_virtual_pointer<volatile uint32_t>(
                GIC_DISTRIBUTOR_BASE + offset
            );
        }

        volatile uint32_t &gicc(a9n::word offset)
        {
            return *a9n::kernel::physical_to_virtual_pointer<volatile uint32_t>(
                GIC_CPU_INTERFACE_BASE + offset
            );
        }
    }

    hal_result init_interrupt_controller()
    {
        a9n::kernel::utility::logger::printh(
            "GICv2: distributor=0x%016llx, CPU-interface=0x%016llx\n",
            GIC_DISTRIBUTOR_BASE,
            GIC_CPU_INTERFACE_BASE
        );
        gicd(0x000) = 0;
        gicc(0x000) = 0;

        interrupt_count = ((gicd(0x004) & 0x1f) + 1) * 32;
        if (interrupt_count > a9n::hal::IRQ_NUMBER_MAX)
        {
            interrupt_count = a9n::hal::IRQ_NUMBER_MAX;
        }
        a9n::kernel::utility::logger::printh(
            "GICv2: configuring %llu interrupt IDs.\n",
            interrupt_count
        );

        for (a9n::word irq = 0; irq < interrupt_count; irq += 32)
        {
            const auto index        = irq / 32;
            gicd(0x080 + index * 4) = 0xffffffff;
            gicd(0x180 + index * 4) = 0xffffffff;
            gicd(0x280 + index * 4) = 0xffffffff;
        }

        for (a9n::word irq = 0; irq < interrupt_count; irq += 4)
        {
            gicd(0x400 + irq) = 0xa0a0a0a0;
            if (irq >= 32)
            {
                gicd(0x800 + irq) = 0x01010101;
            }
        }

        gicd(0x000) = 1;
        asm volatile("dsb sy; isb" ::: "memory");
        return init_current_core_interrupt_controller();
    }

    hal_result init_current_core_interrupt_controller()
    {
        // SGI and PPI state is banked per GICv2 CPU interface.
        gicd(0x080) = 0xffffffff;
        gicd(0x180) = 0xffffffff;
        gicd(0x280) = 0xffffffff;
        for (a9n::word irq = 0; irq < 32; irq += 4)
        {
            gicd(0x400 + irq) = 0xa0a0a0a0;
        }

        gicc(0x000) = 0;
        gicc(0x004) = 0xff;
        gicc(0x008) = 0x3;
        gicc(0x000) = 1;
        asm volatile("dsb sy; isb" ::: "memory");
        a9n::word mpidr {};
        asm volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
        a9n::kernel::utility::logger::printh(
            "GICv2 CPU interface enabled: MPIDR=0x%016llx, priority-mask=0xff\n",
            mpidr
        );
        return {};
    }

    hal_result enable_irq(a9n::word irq_number)
    {
        if (irq_number >= interrupt_count)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }
        gicd(0x100 + (irq_number / 32) * 4) = 1U << (irq_number % 32);
        asm volatile("dsb sy" ::: "memory");
        return {};
    }

    hal_result disable_irq(a9n::word irq_number)
    {
        if (irq_number >= interrupt_count)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }
        gicd(0x180 + (irq_number / 32) * 4) = 1U << (irq_number % 32);
        asm volatile("dsb sy" ::: "memory");
        return {};
    }

    a9n::word acknowledge_irq()
    {
        return gicc(0x00c) & 0x3ff;
    }

    void end_of_interrupt(a9n::word irq_number)
    {
        gicc(0x010) = static_cast<uint32_t>(irq_number);
        asm volatile("dsb sy" ::: "memory");
    }

    hal_result send_sgi(a9n::word sgi, a9n::word core_number)
    {
        if (sgi >= 16 || core_number >= 8)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }
        gicd(0xf00) = static_cast<uint32_t>((1U << core_number) << 16) | sgi;
        asm volatile("dsb sy" ::: "memory");
        return {};
    }
}
