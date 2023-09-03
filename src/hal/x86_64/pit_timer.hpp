#ifndef PIT_TIMER_HPP
#define PIT_TIMER_HPP

#include <interface/timer.hpp>

#include <interface/port_io.hpp>

#include <interface/serial.hpp>

#include "pic.hpp"

#include <stdint.h>

namespace hal::x86_64
{
    class pit_timer final : public hal::interface::timer
    {
        public:
            pit_timer(hal::interface::port_io &injected_port_io);
            ~pit_timer();

            void init_timer() override;
            void configure_timer(uint16_t hz) override;
            void clock() override;
            uint32_t get_tick() override;

            __attribute__((interrupt("IRQ"))) static void handle(void *data);

            uint32_t ticks = 0;

        private:
            static pit_timer *this_timer;
            hal::interface::port_io &_port_io;
            hal::x86_64::pic _pic;
    };
}

#endif
