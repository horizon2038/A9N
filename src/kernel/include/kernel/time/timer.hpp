#ifndef TIMER_HPP
#define TIMER_HPP

#include <hal/interface/timer.hpp>

namespace a9n::kernel
{
    class timer
    {
      public:
        timer(a9n::hal::timer &timer);
        ~timer();

        void clock();
    };
}

#endif
