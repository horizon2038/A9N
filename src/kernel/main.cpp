#include "../hal/x86_64/segment_configurator.hpp"
#include "../hal/x86_64/segment_configurator.hpp"
#include <stdint.h>
#include <cpp_dependent/new.hpp>

#include <interface/interrupt.hpp>
#include "../hal/x86_64/interrupt.hpp"

#include <interface/port_io.hpp>
#include "../hal/x86_64/port_io.hpp"

#include "../hal/x86_64/serial.hpp"

#include "../hal/x86_64/arch_initializer.hpp"

#include "print.hpp"

#include <interface/timer.hpp>
#include "../hal/x86_64/pit_timer.hpp"
#include "process.hpp"

void kernel_main(void);

hal::interface::timer *timer;

extern "C" int kernel_entry()
{
    constexpr uint16_t port_io_size = sizeof(hal::x86_64::port_io);
    alignas(hal::x86_64::port_io) char port_io_buf[port_io_size]; 
    hal::interface::port_io *my_port_io = new((void*)port_io_buf) hal::x86_64::port_io{};

    constexpr uint16_t serial_size = sizeof(hal::x86_64::serial);
    alignas(hal::x86_64::serial) char serial_buf[serial_size];
    hal::interface::serial *my_serial = new((void*)serial_buf) hal::x86_64::serial{*my_port_io};

    my_serial->init_serial(115200);

    my_serial->write_string_serial("@horizon2k38\n");

    my_serial->write_string_serial("[ START ] a9n kernel\n");

    my_serial->write_string_serial("[ INIT ] portio driver\n");
    my_serial->write_string_serial("[ START ] portio driver\n");

    constexpr uint16_t segment_configurator_size = sizeof(hal::x86_64::segment_configurator);
    alignas(hal::x86_64::segment_configurator) char buf[segment_configurator_size];
    hal::x86_64::segment_configurator *my_segment_configurator = new((void*)buf) hal::x86_64::segment_configurator{};
    my_segment_configurator->init_gdt();
    my_serial->write_string_serial("[ INIT ] architecture\n");

    constexpr uint16_t interrupt_size = sizeof(hal::x86_64::interrupt);
    alignas(hal::x86_64::interrupt) char interrupt_buf[interrupt_size];
    hal::interface::interrupt *my_interrupt = new((void*)interrupt_buf) hal::x86_64::interrupt{};
    my_interrupt->init_interrupt();
    my_serial->write_string_serial("[ INIT ] interrupt\n");
    my_serial->write_string_serial("[ START ] interrupt\n");

    constexpr uint16_t print_size = sizeof(kernel::utility::print);
    alignas(kernel::utility::print) char print_buf[print_size];
    kernel::utility::print *my_print = new((void*)print_buf) kernel::utility::print{*my_serial};

    // char buffer[100];
    // my_print->sprintf(buffer, "sprintf test: %d\n", 2038);
    // my_serial->write_string_serial(buffer);

    constexpr uint16_t timer_size = sizeof(hal::x86_64::pit_timer);
    alignas(hal::x86_64::pit_timer) char timer_buf[timer_size];
    // hal::interface::timer *my_timer = new((void*)timer_buf) hal::x86_64::pit_timer{*my_port_io};
    timer = new((void*)timer_buf) hal::x86_64::pit_timer{*my_port_io};

    hal::interface::interrupt_handler timer_interrupt_handler = hal::x86_64::pit_timer::handle;

    my_interrupt->disable_interrupt_all();
    my_serial->write_string_serial("[ DISABLE ] interrupt\n");
    my_interrupt->register_interrupt(0, timer_interrupt_handler);
    my_serial->write_string_serial("[ REGISTER ] timer interrupt\n");
    my_interrupt->enable_interrupt_all();
    my_serial->write_string_serial("[ ENABLE ] interrupt\n");

    timer->init_timer();
    my_serial->write_string_serial("[ INIT ] timer\n");
    my_serial->write_string_serial("[ START ] timer\n");

    my_serial->write_string_serial("[ INIT ] process_manager\n");

    kernel_main();

    return 2038;
}

void kernel_main(void)
{
    constexpr uint16_t port_io_size = sizeof(hal::x86_64::port_io);
    alignas(hal::x86_64::port_io) char port_io_buf[port_io_size]; 
    hal::interface::port_io *my_port_io = new((void*)port_io_buf) hal::x86_64::port_io{};

    constexpr uint16_t serial_size = sizeof(hal::x86_64::serial);
    alignas(hal::x86_64::serial) char serial_buf[serial_size];
    hal::interface::serial *my_serial = new((void*)serial_buf) hal::x86_64::serial{*my_port_io};

    constexpr uint16_t print_size = sizeof(kernel::utility::print);
    alignas(kernel::utility::print) char print_buf[print_size];
    kernel::utility::print *my_print = new((void*)print_buf) kernel::utility::print{*my_serial};

    my_serial->init_serial(115200);
    my_serial->write_string_serial("[ RUN ] kernel_main\n");
    char tick_buffer[100];
    while(1)
    {
        my_print->sprintf(tick_buffer, "tick: %d\n", timer->get_tick());
        my_serial->write_string_serial(tick_buffer);
        asm volatile ("hlt");
    }

}

