#ifndef HAL_HPP
#define HAL_HPP

// core services
#include <hal/interface/memory_manager.hpp>
#include <hal/interface/process_manager.hpp>
#include <hal/interface/interrupt.hpp>

// platform services
#include <hal/interface/arch_initializer.hpp>

// peripheral drivers
#include <hal/interface/timer.hpp>
#include <hal/interface/port_io.hpp>
#include <hal/interface/serial.hpp>

namespace a9n::hal
{
    struct hal
    {
        memory_manager  *_memory_manager;
        process_manager *_process_manager;
        interrupt       *_interrupt;

        // platform services
        arch_initializer *_arch_initializer;

        // peripheral drivers
        port_io *_port_io;
        serial  *_serial;
        timer   *_timer;
    };
}

#endif
