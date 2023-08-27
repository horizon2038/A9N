#ifndef PIT_TIMER_HPP
#define PIT_TIMER_HPP

#include "interface/interrupt_handler.hpp"
#include <interface/timer.hpp>

#include <interface/port_io.hpp>

#include <interface/serial.hpp>

#include <stdint.h>

namespace hal::x86_64
{
    class pit_timer final : public hal::interface::timer, public hal::interface::interrupt_handler
    {
        public:
            pit_timer(hal::interface::port_io &injected_port_io, hal::interface::serial &injected_serial);
            ~pit_timer();
            void init_timer() override;
            void configure_timer(uint16_t hz) override;
            void handle() override;
            uint32_t ticks = 0;

        private:
            hal::interface::port_io &_port_io;
            hal::interface::serial &_serial;
    };
}

#endif
