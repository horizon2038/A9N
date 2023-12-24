#ifndef HAL_TIMER_HPP
#define HAL_TIMER_HPP

#include <stdint.h>
#include <library/common/types.hpp>

namespace hal::interface
{
    class timer
    {
      public:
        virtual void init_timer() = 0;
        virtual void configure_timer(uint16_t hz) = 0;
        virtual void clock();
        virtual common::word get_tick();
    };
}

#endif
