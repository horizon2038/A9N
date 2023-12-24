#ifndef INTERRUPT_MANAGER_HPP
#define INTERRUPT_MANAGER_HPP

#include <stdint.h>

#include <interface/interrupt.hpp>

namespace kernel
{
    class interrupt_manager
    {
        public:
            interrupt_manager(hal::interface::interrupt &target_interrupt);
            ~interrupt_manager();

            void init();

            void enable_interrupt_all();
            void disable_interrupt_all();
            void ack_interrupt();

        private:
            hal::interface::interrupt &_interrupt;

            void init_handler();
    };

    extern "C" void handle_timer();
}

#endif
