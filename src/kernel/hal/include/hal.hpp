#ifndef HAL_HPP
#define HAL_HPP

#include "interrupt.hpp"

#include "architecture_initializer.hpp"

#include "serial.hpp"


namespace hal
{
    class hal
    {
        public:
            virtual interrupt &instance_interrupt() = 0;
            virtual architecture_initializer &instance_architecture_initializer() = 0;
            virtual serial &instance_serial() = 0;
    };
}

#endif
