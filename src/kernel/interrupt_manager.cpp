#include "interrupt_manager.hpp"

#include "kernel.hpp"

#include <library/logger.hpp>

namespace kernel
{
    interrupt_manager::interrupt_manager(hal::interface::interrupt &target_interrupt)
        : _interrupt(target_interrupt)
    {
    }

    void interrupt_manager::init()
    {
        _interrupt.init_interrupt();
        init_handler();
    }

    void interrupt_manager::init_handler()
    {
        _interrupt.register_handler(0, handle_timer);
    }

    void interrupt_manager::enable_interrupt_all()
    {
        _interrupt.enable_interrupt_all();
    }

    void interrupt_manager::disable_interrupt_all()
    {
        _interrupt.disable_interrupt_all();
    }

    void interrupt_manager::ack_interrupt()
    {
        _interrupt.ack_interrupt();
    }

    // this handler is required
    extern "C" void handle_timer()
    {
        // kernel::utility::logger::printk("handle_timer\n");
        kernel::kernel_object::interrupt_manager->ack_interrupt();
        kernel::kernel_object::process_manager->switch_context();

    }
}
