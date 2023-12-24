#ifndef TIMER_HPP
#define TIMER_HPP

#include <hal/interface/timer.hpp>

namespace kernel
{
    class timer
    {
      public:
        timer(hal::interface::timer &timer);
        ~timer();

        void clock();
    };
}

#endif
