#ifndef HAL_HPP
#define HAL_HPP

// core services
#include "arch_context_switch.hpp"
#include "interrupt.hpp"

// platform services
#include "arch_initializer.hpp"

// peripheral drivers
#include "timer.hpp"
#include "serial.hpp"


namespace hal::interface
{
    class hal
    {
        public:
            // core services
            virtual arch_context_switch &_arch_context_switch() = 0;
            virtual interrupt &_interrupt() = 0;

            // platform services
            virtual arch_initializer &_initializer() = 0;

            // peripheral drivers
            virtual serial &_serial() = 0;
            virtual timer &_timer() = 0; 
    };
}

#endif
