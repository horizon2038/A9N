#include <hal/x86_64/factory/hal_factory.hpp>

#include <hal/interface/arch_initializer.hpp>
#include <hal/interface/process_manager.hpp>
#include <hal/interface/memory_manager.hpp>
#include <hal/interface/process_manager.hpp>

#include <hal/x86_64/time/pit_timer.hpp>
#include <hal/x86_64/io/port_io.hpp>

#include <liba9n/cpp_dependent/new.hpp>
#include <liba9n/libc/string.hpp>

namespace a9n::hal::x86_64
{
    alignas(a9n::hal::hal) char hal_factory::hal_buffer[hal_factory::hal_size]
        = {};

    alignas(a9n::hal::x86_64::memory_manager
    ) char hal_factory::memory_manager_buffer[hal_factory::memory_manager_size]
        = {};
    alignas(a9n::hal::x86_64::process_manager
    ) char hal_factory::process_manager_buffer[hal_factory::process_manager_size]
        = {};
    alignas(a9n::hal::x86_64::interrupt
    ) char hal_factory::interrupt_buffer[hal_factory::interrupt_size]
        = {};

    alignas(a9n::hal::x86_64::arch_initializer) char hal_factory::
        arch_initializer_buffer[hal_factory::arch_initializer_size]
        = {};

    alignas(a9n::hal::x86_64::port_io
    ) char hal_factory::port_io_buffer[hal_factory::port_io_size]
        = {};
    alignas(a9n::hal::x86_64::serial
    ) char hal_factory::serial_buffer[hal_factory::serial_size]
        = {};
    alignas(a9n::hal::x86_64::pit_timer
    ) char hal_factory::timer_buffer[hal_factory::timer_size]
        = {};

    a9n::hal::hal *hal_factory::make()
    {
        a9n::hal::hal *hal_pointer = new (hal_buffer) a9n::hal::hal();

        a9n::hal::memory_manager *memory_manager_pointer
            = new (memory_manager_buffer) memory_manager();
        a9n::hal::process_manager *process_manager_pointer
            = new (process_manager_buffer) process_manager();
        a9n::hal::interrupt *interrupt_pointer
            = new (interrupt_buffer) interrupt();

        a9n::hal::arch_initializer *arch_initializer_pointer
            = new (arch_initializer_buffer) arch_initializer();

        a9n::hal::port_io *port_io_pointer = new (port_io_buffer) port_io();
        a9n::hal::serial  *serial_pointer
            = new (serial_buffer) serial(*port_io_pointer);
        a9n::hal::timer *timer_pointer = new (timer_buffer) pit_timer();

        hal_pointer->_memory_manager   = memory_manager_pointer;
        hal_pointer->_process_manager  = process_manager_pointer;
        hal_pointer->_interrupt        = interrupt_pointer;
        hal_pointer->_arch_initializer = arch_initializer_pointer;
        hal_pointer->_port_io          = port_io_pointer;
        hal_pointer->_serial           = serial_pointer;
        hal_pointer->_timer            = timer_pointer;

        return hal_pointer;
    }
}
