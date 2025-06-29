#ifndef A9N_KERNEL_IRQ_NOTIFICATION_HANDLERS_HPP
#define A9N_KERNEL_IRQ_NOTIFICATION_HANDLERS_HPP

#include <hal/arch/arch_types.hpp>
#include <kernel/capability/interrupt_port.hpp>
#include <liba9n/libcxx/array>

namespace a9n::kernel
{
    inline liba9n::std::array<capability_slot, hal::IRQ_NUMBER_MAX> irq_notification_handlers;

    inline constexpr liba9n::result<liba9n::not_null<capability_slot>, kernel_error>
        irq_number_to_handler(a9n::word irq_number)
    {
        if (irq_number >= hal::IRQ_NUMBER_MAX)
        {
            return kernel_error::UNSUPPORTED;
        }

        return irq_notification_handlers[irq_number];
    };

    inline constexpr liba9n::result<liba9n::not_null<capability_slot>, kernel_error>
        has_irq_handler_notification(liba9n::not_null<capability_slot> handler)
    {
        if (handler->type != capability_type::NOTIFICATION_PORT)
        {
            return kernel_error::INIT_FIRST;
        }

        return handler;
    }
}

#endif
