#include <hal/aarch64/platform.hpp>

#include <hal/arch/arch_types.hpp>
#include <kernel/memory/memory.hpp>

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
        gicd(0x000)     = 0;
        gicc(0x000)     = 0;

        interrupt_count = ((gicd(0x004) & 0x1f) + 1) * 32;
        if (interrupt_count > a9n::hal::IRQ_NUMBER_MAX)
        {
            interrupt_count = a9n::hal::IRQ_NUMBER_MAX;
        }

        for (a9n::word irq = 0; irq < interrupt_count; irq += 32)
        {
            const auto index        = irq / 32;
            gicd(0x080 + index * 4) = 0xffffffff; // Group 1/non-secure.
            gicd(0x180 + index * 4) = 0xffffffff; // Disable.
            gicd(0x280 + index * 4) = 0xffffffff; // Clear pending.
        }

        for (a9n::word irq = 0; irq < interrupt_count; irq += 4)
        {
            gicd(0x400 + irq) = 0xa0a0a0a0;
            if (irq >= 32)
            {
                gicd(0x800 + irq) = 0x01010101; // Route SPIs to CPU interface 0.
            }
        }

        gicc(0x004) = 0xff; // Priority mask.
        gicc(0x008) = 0x3;  // No priority grouping.
        gicc(0x000) = 1;
        gicd(0x000) = 1;
        asm volatile("dsb sy; isb" ::: "memory");
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
