#ifndef X86_64_PIT_TIMER_HPP
#define X86_64_PIT_TIMER_HPP

#include "hal/hal_result.hpp"
#include <hal/interface/timer.hpp>

#include <hal/interface/port_io.hpp>
#include <hal/interface/serial.hpp>

#include <hal/x86_64/interrupt/pic.hpp>

#include <stdint.h>

namespace a9n::hal::x86_64
{
    class pit_timer final : public a9n::hal::timer
    {
      public:
        hal_result init() override;
        hal_result configure_cycle(uint16_t hz) override;

      private:
        static pit_timer         *this_timer;
        a9n::hal::x86_64::port_io _port_io;
    };
}

#endif
