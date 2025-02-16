#ifndef INTERRUPT_MANAGER_HPP
#define INTERRUPT_MANAGER_HPP

#include <stdint.h>

#include <hal/interface/interrupt.hpp>
#include <kernel/kernel_result.hpp>

namespace a9n::kernel
{
    class interrupt_manager
    {
      public:
        kernel_result init(void);

        void enable_interrupt_all();
        void disable_interrupt_all();
        void ack_interrupt();

      private:
        void init_handler();
    };

    inline interrupt_manager interrupt_manager_core {};

    extern "C" void handle_timer();
    extern "C" void handle_interrupt(a9n::word irq_number);
    extern "C" void handle_fault(
        a9n::kernel::fault_type type,
        a9n::sword              fault_code,
        a9n::virtual_address    fault_address
    );
}

#endif
