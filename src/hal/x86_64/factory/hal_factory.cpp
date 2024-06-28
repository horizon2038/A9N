#include <hal/x86_64/factory/hal_factory.hpp>

#include <hal/interface/arch_initializer.hpp>
#include <hal/interface/process_manager.hpp>
#include <hal/interface/memory_manager.hpp>
#include <hal/interface/process_manager.hpp>

#include <hal/x86_64/time/pit_timer.hpp>
#include <hal/x86_64/io/port_io.hpp>

#include <library/cpp_dependent/new.hpp>
#include <library/libc/string.hpp>

namespace hal::x86_64
{
    alignas(hal::interface::hal
    ) char hal_factory::hal_buffer[hal_factory::hal_size]
        = {};

    alignas(hal::x86_64::memory_manager
    ) char hal_factory::memory_manager_buffer[hal_factory::memory_manager_size]
        = {};
    alignas(hal::x86_64::process_manager
    ) char hal_factory::process_manager_buffer[hal_factory::process_manager_size]
        = {};
    alignas(hal::x86_64::interrupt
    ) char hal_factory::interrupt_buffer[hal_factory::interrupt_size]
        = {};

    alignas(hal::x86_64::arch_initializer) char hal_factory::
        arch_initializer_buffer[hal_factory::arch_initializer_size]
        = {};

    alignas(hal::x86_64::port_io
    ) char hal_factory::port_io_buffer[hal_factory::port_io_size]
        = {};
    alignas(hal::x86_64::serial
    ) char hal_factory::serial_buffer[hal_factory::serial_size]
        = {};
    alignas(hal::x86_64::pit_timer
    ) char hal_factory::timer_buffer[hal_factory::timer_size]
        = {};

    hal::interface::hal *hal_factory::make()
    {
        hal::interface::hal *hal_pointer
            = new (hal_buffer) hal::interface::hal();

        hal::interface::memory_manager *memory_manager_pointer
            = new (memory_manager_buffer) memory_manager();
        hal::interface::process_manager *process_manager_pointer
            = new (process_manager_buffer) process_manager();
        hal::interface::interrupt *interrupt_pointer
            = new (interrupt_buffer) interrupt();

        hal::interface::arch_initializer *arch_initializer_pointer
            = new (arch_initializer_buffer) arch_initializer();

        hal::interface::port_io *port_io_pointer
            = new (port_io_buffer) port_io();
        hal::interface::serial *serial_pointer
            = new (serial_buffer) serial(*port_io_pointer);
        hal::interface::timer *timer_pointer = new (timer_buffer) pit_timer();

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
