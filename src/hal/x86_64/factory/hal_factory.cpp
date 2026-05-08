#include <hal/x86_64/factory/hal_factory.hpp>

#include <hal/interface/arch_initializer.hpp>
#include <hal/interface/memory_manager.hpp>
#include <hal/interface/process_manager.hpp>

#include <hal/x86_64/arch/cpu.hpp>
#include <hal/x86_64/io/port_io.hpp>
#include <hal/x86_64/time/local_apic_timer.hpp>

#include <liba9n/libc/string.hpp>
#include <liba9n/libcxx/new>

namespace a9n::hal::x86_64
{
    alignas(a9n::hal::hal) char hal_factory::hal_buffer[hal_factory::hal_size] = {};

    alignas(a9n::hal::x86_64::port_io) char hal_factory::port_io_buffer[hal_factory::port_io_size] = {};
    alignas(a9n::hal::x86_64::serial) char hal_factory::serial_buffer[hal_factory::serial_size] = {};

    a9n::hal::hal *hal_factory::make()
    {
        a9n::hal::hal *hal_pointer         = new (hal_buffer) a9n::hal::hal();

        a9n::hal::port_io *port_io_pointer = new (port_io_buffer) port_io();
        a9n::hal::serial  *serial_pointer  = new (serial_buffer) serial(*port_io_pointer);
        a9n::hal::timer   *timer_pointer   = &local_apic_timer_core;

        hal_pointer->_arch_initializer     = &arch_initializer_core;
        hal_pointer->_port_io              = port_io_pointer;
        hal_pointer->_serial               = serial_pointer;
        hal_pointer->_timer                = timer_pointer;

        return hal_pointer;
    }
}
