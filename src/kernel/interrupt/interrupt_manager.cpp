#include "hal/hal_result.hpp"
#include <kernel/interrupt/interrupt_manager.hpp>

#include <kernel/kernelcall/kernel_call.hpp>
#include <kernel/process/process_manager.hpp>

#include <kernel/utility/logger.hpp>

namespace a9n::kernel
{
    __attribute__((weak)) extern "C" void *memset(void *buffer, char value, size_t buffer_size)
    {
        uint8_t *buffer_pointer = reinterpret_cast<uint8_t *>(buffer);

        for (size_t i = 0; i < buffer_size; i++)
        {
            *buffer_pointer = value;
            buffer_pointer++;
        }

        return buffer;
    }

    kernel_result interrupt_manager::init(void)
    {
        init_handler();

        return {};
    }

    void interrupt_manager::init_handler(void)
    {
        a9n::kernel::utility::logger::printk("init interrupt handlers ...\n");
        a9n::hal::register_system_timer_handler(handle_timer);
        a9n::hal::register_kernel_call_handler(handle_kernel_call);
    }

    void interrupt_manager::enable_interrupt_all()
    {
        a9n::kernel::utility::logger::printk("enable interrupt ...\n");
        a9n::hal::enable_interrupt_all();
    }

    void interrupt_manager::disable_interrupt_all()
    {
        a9n::kernel::utility::logger::printk("disable interrupt ...\n");
        a9n::hal::disable_interrupt_all();
    }

    void interrupt_manager::ack_interrupt()
    {
        a9n::hal::ack_interrupt();
    }

    // this handler is required
    extern "C" void handle_timer()
    {
        auto result = a9n::kernel::process_manager_core.switch_context().and_then(
            [](void) -> kernel_result
            {
                a9n::kernel::interrupt_manager_core.ack_interrupt();
                return {};
            }
        );
    }

}
