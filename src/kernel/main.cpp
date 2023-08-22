#include <x86_64/segment_configurator.hpp>
#include <stdint.h>
#include <cpp_dependent/new.hpp>

#include <include/interrupt.hpp>
#include <x86_64/x86_64_interrupt.hpp>

#include <include/port_io.hpp>
#include <x86_64/x86_64_port_io.hpp>

#include <x86_64/x86_64_serial.hpp>

#include "x86_64/x86_64_architecture_initializer.hpp"
#include "x86_64/x86_64_hal.hpp"
#include <hal.hpp>

extern "C" int kernel_main()
{
    constexpr uint16_t port_io_size = sizeof(hal::x86_64::port_io);
    alignas(hal::x86_64::port_io) char port_io_buf[port_io_size]; 
    hal::port_io *my_port_io = new((void*)port_io_buf) hal::x86_64::port_io{};

    constexpr uint16_t serial_size = sizeof(hal::x86_64::serial);
    alignas(hal::x86_64::serial) char serial_buf[serial_size];
    hal::serial *my_serial = new((void*)serial_buf) hal::x86_64::serial{*my_port_io};

    my_serial->init_serial(115200);
    my_serial->write_string_serial("start kernel\n");
    my_serial->write_string_serial("init_serial_driver\n");
    
    constexpr uint16_t segment_configurator_size = sizeof(hal::x86_64::segment_configurator);
    alignas(hal::x86_64::segment_configurator) char buf[segment_configurator_size];
    hal::x86_64::segment_configurator *my_segment_configurator = new((void*)buf) hal::x86_64::segment_configurator{};
    my_segment_configurator->init_gdt();
    my_serial->write_string_serial("init_segment\n");

    constexpr uint16_t interrupt_size = sizeof(hal::x86_64::interrupt);
    alignas(hal::x86_64::interrupt) char interrupt_buf[interrupt_size];
    hal::interrupt *my_interrupt = new((void*)interrupt_buf) hal::x86_64::interrupt{};
    my_interrupt->init_interrupt();
    my_serial->write_string_serial("init_interrupt\n");

    asm volatile("hlt");
    return 2038;
}
