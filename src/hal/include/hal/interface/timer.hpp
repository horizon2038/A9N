#ifndef HAL_TIMER_HPP
#define HAL_TIMER_HPP

#include <stdint.h>
#include <liba9n/common/types.hpp>

namespace a9n::hal
{
    class timer
    {
      public:
        virtual void         init_timer()                 = 0;
        virtual void         configure_timer(uint16_t hz) = 0;
        virtual void         clock();
        virtual a9n::word get_tick();
    };
}

#endif
