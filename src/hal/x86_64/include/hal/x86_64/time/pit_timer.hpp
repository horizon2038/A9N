#ifndef X86_64_PIT_TIMER_HPP
#define X86_64_PIT_TIMER_HPP

#include <hal/interface/timer.hpp>

#include <hal/interface/port_io.hpp>
#include <hal/interface/serial.hpp>

#include <hal/x86_64/interrupt/pic.hpp>

#include <stdint.h>

namespace hal::x86_64
{
    class pit_timer final : public hal::interface::timer
    {
      public:
        pit_timer();
        ~pit_timer();

        void         init_timer() override;
        void         configure_timer(uint16_t hz) override;
        void         clock() override;
        common::word get_tick() override;

        __attribute__((interrupt("IRQ"))) static void handle(void *data);

        uint32_t ticks = 0;

      private:
        static pit_timer    *this_timer;
        hal::x86_64::port_io _port_io;
    };
}

#endif
