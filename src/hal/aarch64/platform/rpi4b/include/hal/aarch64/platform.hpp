#ifndef A9N_HAL_AARCH64_RPI4B_PLATFORM_HPP
#define A9N_HAL_AARCH64_RPI4B_PLATFORM_HPP

#include <hal/aarch64/arch/cpu.hpp>
#include <hal/hal_result.hpp>
#include <kernel/types.hpp>

namespace a9n::hal::aarch64::platform
{
    inline constexpr const char           *PLATFORM_NAME          = "rpi4b";
    inline constexpr a9n::physical_address GPIO_BASE              = 0xfe200000;
    inline constexpr a9n::physical_address UART_BASE              = 0xfe201000;
    inline constexpr a9n::physical_address GIC_DISTRIBUTOR_BASE   = 0xff841000;
    inline constexpr a9n::physical_address GIC_CPU_INTERFACE_BASE = 0xff842000;
    inline constexpr a9n::word             GENERIC_TIMER_IRQ      = 30;

    hal_result init_interrupt_controller();
    hal_result init_current_core_interrupt_controller();
    hal_result enable_irq(a9n::word irq_number);
    hal_result disable_irq(a9n::word irq_number);
    a9n::word  acknowledge_irq();
    void       end_of_interrupt(a9n::word irq_number);
    hal_result send_sgi(a9n::word sgi, a9n::word core_number);
    hal_result start_cpu(const cpu_description &cpu, a9n::physical_address entry_address);

    hal_result init_system_timer();
    hal_result configure_timer(uint16_t hz);
    hal_result start_system_timer();
    void       rearm_system_timer();
}

#endif
