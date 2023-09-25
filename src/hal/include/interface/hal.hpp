#ifndef HAL_HPP
#define HAL_HPP

// core services
#include "process_manager.hpp"
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
            virtual process_manager &_process_manager() = 0;
            virtual interrupt &_interrupt() = 0;

            // platform services
            virtual arch_initializer &_initializer() = 0;

            // peripheral drivers
            virtual serial &_serial() = 0;
            virtual timer &_timer() = 0; 
    };
}

#endif
