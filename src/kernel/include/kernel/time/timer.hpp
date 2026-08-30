#ifndef TIMER_HPP
#define TIMER_HPP

#include <hal/interface/timer.hpp>
#include <stdint.h>

namespace a9n::kernel
{
    inline constexpr uint16_t SYSTEM_CLOCK_FREQUENCY = 250;

    class timer
    {
      public:
        void clock();
    };
}

#endif
