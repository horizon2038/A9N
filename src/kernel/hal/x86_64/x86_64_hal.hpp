#ifndef X86_64_HAL_HPP
#define X86_64_HAL_HPP

#include "../include/hal.hpp"
#include "include/architecture_initializer.hpp"

namespace hal::x86_64
{
    typedef struct
    {
        interrupt &instance_interrupt;
    } core_services;

    typedef struct
    {
        architecture_initializer &instance_architecture_initializer;
    } platform_services;

    typedef struct
    {
        serial &instance_serial;
    } peripheral_drivers;

    class x86_64_hal : hal::hal
    {
        public:
            x86_64_hal(interrupt &injected_interrupt, architecture_initializer &injected_architecture_initializer, serial &injected_serial);
            ~x86_64_hal();
            interrupt &instance_interrupt() override;
            architecture_initializer &instance_architecture_initializer() override;
            serial &instance_serial() override;

        private:
            interrupt &_interrupt;
            architecture_initializer &_architecture_initializer;
            serial &_serial;

    };
}



#endif
