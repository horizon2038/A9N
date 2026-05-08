#ifndef A9N_KERNEL_IRQ_NOTIFICATION_HANDLERS_HPP
#define A9N_KERNEL_IRQ_NOTIFICATION_HANDLERS_HPP

#include <hal/arch/arch_types.hpp>
#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/notification_port.hpp>

#include <liba9n/common/not_null.hpp>
#include <liba9n/libcxx/array>
#include <liba9n/option/option.hpp>

namespace a9n::kernel
{
    struct irq_notification_handler
    {
        capability_slot slot;
        a9n::word       irq_number { 0 };
        bool            used { false };
    };

    inline liba9n::std::array<irq_notification_handler, hal::IRQ_NUMBER_MAX> irq_notification_handlers;

    inline constexpr liba9n::result<liba9n::not_null<irq_notification_handler>, kernel_error>
        irq_number_to_irq_notification_handler(a9n::word irq_number)
    {
        if (irq_number >= hal::IRQ_NUMBER_MAX)
        {
            return kernel_error::UNSUPPORTED;
        }

        return irq_notification_handlers[irq_number];
    };
}

#endif
