#ifndef X86_64_PIT_TIMER_HPP
#define X86_64_PIT_TIMER_HPP

#include "hal/hal_result.hpp"
#include <hal/x86_64/interrupt/pic.hpp>

#include <stdint.h>

namespace a9n::hal::x86_64
{
    class pit_timer final
    {
      public:
        hal_result init();
        hal_result configure_cycle(uint16_t hz);

      private:
        static pit_timer         *this_timer;
        a9n::hal::x86_64::port_io _port_io;
    };
}

#endif
