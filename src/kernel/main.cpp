#include "../hal/x86_64/segment_configurator.hpp"
#include "../hal/x86_64/segment_configurator.hpp"
#include <stdint.h>
#include <cpp_dependent/new.hpp>

#include <interface/interrupt.hpp>
#include "../hal/x86_64/x86_64_interrupt.hpp"

#include <interface/port_io.hpp>
#include "../hal/x86_64/x86_64_port_io.hpp"

#include "../hal/x86_64/x86_64_serial.hpp"

#include "../hal/x86_64/x86_64_architecture_initializer.hpp"

#include "print.hpp"

extern "C" int kernel_main()
{
    constexpr uint16_t port_io_size = sizeof(hal::x86_64::port_io);
    alignas(hal::x86_64::port_io) char port_io_buf[port_io_size]; 
    hal::interface::port_io *my_port_io = new((void*)port_io_buf) hal::x86_64::port_io{};

    constexpr uint16_t serial_size = sizeof(hal::x86_64::serial);
    alignas(hal::x86_64::serial) char serial_buf[serial_size];
    hal::interface::serial *my_serial = new((void*)serial_buf) hal::x86_64::serial{*my_port_io};

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
    hal::interface::interrupt *my_interrupt = new((void*)interrupt_buf) hal::x86_64::interrupt{};
    my_interrupt->init_interrupt();
    my_serial->write_string_serial("init_interrupt\n");

    my_serial->write_string_serial("a9n_kernel\n");
    my_serial->write_string_serial("mitou jr project\n");
    my_serial->write_string_serial("@horizon2k38\n");

    constexpr uint16_t print_size = sizeof(kernel::utility::print);
    alignas(kernel::utility::print) char print_buf[print_size];
    kernel::utility::print *my_print = new((void*)print_buf) kernel::utility::print{*my_serial};

    char buffer[100];
    my_print->sprintf(buffer, "sprintf test: %d\n", 2038);
    my_serial->write_string_serial(buffer);

    asm volatile("hlt");
    return 2038;
}
