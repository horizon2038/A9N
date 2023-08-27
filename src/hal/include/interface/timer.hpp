#ifndef TIMER_HPP
#define TIMER_HPP

#include <stdint.h>

namespace hal::interface
{
    class timer
    {
        public:
            virtual void init_timer() = 0;
            virtual void configure_timer(uint16_t hz) = 0;
    };
}

#endif
