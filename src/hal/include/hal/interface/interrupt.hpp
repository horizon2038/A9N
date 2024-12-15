#ifndef HAL_INTERRUPT_HPP
#define HAL_INTERRUPT_HPP

#include <hal/hal_result.hpp>
#include <kernel/kernelcall/kernel_call.hpp>
#include <kernel/types.hpp>
#include <stdint.h>

namespace a9n::hal
{

    using interrupt_handler   = void (*)();
    using kernel_call_handler = void (*)(a9n::kernel::kernel_call_type type);

    enum class interrupt_type
    {
        INTERRUPT,
        EXCEPTION
    };

    // TODO: devirtualization
    // register some handlers
    hal_result register_interrupt_handler(a9n::word irq_number, interrupt_handler handler);
    hal_result register_system_timer_handler(interrupt_handler handler);
    hal_result register_kernel_call_handler(kernel_call_handler handler);

    // enable/disable irq
    hal_result enable_interrupt(a9n::word irq_number);
    hal_result disable_interrupt(a9n::word irq_number);
    hal_result enable_interrupt_all(void);
    hal_result disable_interrupt_all(void);

    // notify
    hal_result ack_interrupt(void);

}
#endif
