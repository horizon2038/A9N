#ifndef HAL_TIMER_HPP
#define HAL_TIMER_HPP

#include <hal/hal_result.hpp>
#include <kernel/types.hpp>
#include <stdint.h>

namespace a9n::hal
{
    // for system clock
    class timer
    {
      public:
        virtual hal_result init()                       = 0;
        virtual hal_result configure_cycle(uint16_t hz) = 0;
    };

    hal_result configure_system_clock_frequency(uint16_t hz);

}

#endif
