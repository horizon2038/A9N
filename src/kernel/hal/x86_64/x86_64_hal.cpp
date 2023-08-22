#include "x86_64_hal.hpp"
#include "architecture_initializer.hpp"

namespace hal::x86_64
{
    x86_64_hal::x86_64_hal(interrupt &injected_interrupt, architecture_initializer &injected_architecture_initializer, serial &injected_serial) :
        _interrupt(injected_interrupt),
        _architecture_initializer(injected_architecture_initializer),
        _serial(injected_serial)
    {
    };

    x86_64_hal::~x86_64_hal()
    {
    };

    interrupt &x86_64_hal::instance_interrupt()
    {
        return _interrupt;
    }
    
    architecture_initializer &x86_64_hal::instance_architecture_initializer()
    {
        return _architecture_initializer;
    }

    serial &x86_64_hal::instance_serial()
    {
        return _serial;
    }

}
