#ifndef HAL_HPP
#define HAL_HPP

// core services
#include <interface/memory_manager.hpp>
#include <interface/process_manager.hpp>
#include <interface/interrupt.hpp>

// platform services
#include <interface/arch_initializer.hpp>

// peripheral drivers
#include <interface/timer.hpp>
#include <interface/port_io.hpp>
#include <interface/serial.hpp>


namespace hal::interface
{
    struct hal
    {
        memory_manager *_memory_manager;
        process_manager *_process_manager;
        interrupt *_interrupt;

        // platform services
        arch_initializer *_arch_initializer;

        // peripheral drivers
        port_io *_port_io;
        serial *_serial;
        timer *_timer; 
    };
}

#endif
