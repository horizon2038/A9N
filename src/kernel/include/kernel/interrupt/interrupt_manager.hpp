#ifndef INTERRUPT_MANAGER_HPP
#define INTERRUPT_MANAGER_HPP

#include <stdint.h>

#include <hal/interface/interrupt.hpp>

namespace a9n::kernel
{
    class interrupt_manager
    {
      public:
        interrupt_manager(a9n::hal::interrupt &target_interrupt);
        ~interrupt_manager();

        void init();

        void enable_interrupt_all();
        void disable_interrupt_all();
        void ack_interrupt();

      private:
        a9n::hal::interrupt &_interrupt;

        void init_handler();
    };

    extern "C" void handle_timer();
}

#endif
