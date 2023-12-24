#ifndef HAL_TIMER_HPP
#define HAL_TIMER_HPP

#include <stdint.h>

namespace hal::interface
{
    class timer
    {
        public:
            virtual void init_timer() = 0;
            virtual void configure_timer(uint16_t hz) = 0;
            virtual void clock();
            virtual uint32_t get_tick();
    };
}

#endif
