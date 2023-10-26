#include "../hal/x86_64/segment_configurator.hpp"
#include <stdint.h>
#include <cpp_dependent/new.hpp>

#include <interface/interrupt.hpp>
#include "../hal/x86_64/interrupt.hpp"

#include <interface/port_io.hpp>
#include "../hal/x86_64/port_io.hpp"

#include "../hal/x86_64/serial.hpp"

// #include "../hal/x86_64/arch_initializer.hpp"

#include <library/print.hpp>
#include <library/logger.hpp>

#include <interface/timer.hpp>
#include "../hal/x86_64/pit_timer.hpp"
#include "common.hpp"
#include "process.hpp"

#include "boot_info.h"

#include "memory_manager.hpp"
#include <interface/memory_manager.hpp>
#include "../hal/x86_64/memory_manager.hpp"

#include <interface/hal.hpp>
#include <interface/hal_factory.hpp>
#include "../hal/x86_64/hal_factory.hpp"

#include "kernel.hpp"

void kernel_main(void);


hal::interface::hal *hal_instance;

constexpr uint32_t hal_factory_size = sizeof(hal::x86_64::hal_factory);
alignas(hal::x86_64::hal_factory) static char hal_factory_buffer[hal_factory_size];

extern "C" int kernel_entry(boot_info *target_boot_info)
{
    // make HAL and kernel objects.
    hal::interface::hal_factory *hal_factory_instance = new (hal_factory_buffer) hal::x86_64::hal_factory();
    hal_instance = hal_factory_instance->make();

    constexpr uint16_t logger_size = sizeof(kernel::utility::logger);
    alignas(kernel::utility::logger) char logger_buf[logger_size];
    kernel::utility::logger *my_logger = new((void*)logger_buf) kernel::utility::logger{*hal_instance->_serial};
    using logger = kernel::utility::logger;

    logger::a9nout();
    logger::printk("start A9N kernel\n");
    logger::split();

    logger::printk("kernel_entry_address\e[52G:\e[60G0x%016llx\n", reinterpret_cast<uint64_t>(kernel_entry));
    logger::split();

    logger::printk("init hal\n");
    logger::printk("init port_io\n");
    logger::printk("init serial\n");
    logger::printk("init logger\n");

    logger::printk("create memory_manager\n");

    kernel::kernel_object::memory_manager = new(kernel::kernel_object::memory_manager_buffer) kernel::memory_manager(*hal_instance->_memory_manager, target_boot_info->boot_memory_info);
    kernel::kernel_object::memory_manager->allocate_physical_memory(40, nullptr);
    kernel::kernel_object::memory_manager->map_virtual_memory(nullptr, 0xffff800200000000, 0x0000, 3);

    logger::printk("init architecture\n");
    hal_instance->_arch_initializer->init_architecture();

    constexpr uint16_t interrupt_size = sizeof(hal::x86_64::interrupt);
    alignas(hal::x86_64::interrupt) char interrupt_buf[interrupt_size];
    hal::interface::interrupt *my_interrupt = new((void*)interrupt_buf) hal::x86_64::interrupt{};
    my_interrupt->init_interrupt();

    logger::printk("init interrupt\n");

    hal::interface::interrupt_handler timer_interrupt_handler = hal::x86_64::pit_timer::handle;

    logger::printk("init process_manager\n");

    kernel_main();

    return 2038;
}

void kernel_main(void)
{
    using logger = kernel::utility::logger;
    constexpr uint16_t port_io_size = sizeof(hal::x86_64::port_io);
    alignas(hal::x86_64::port_io) char port_io_buf[port_io_size]; 
    hal::interface::port_io *my_port_io = new((void*)port_io_buf) hal::x86_64::port_io{};

    constexpr uint16_t serial_size = sizeof(hal::x86_64::serial);
    alignas(hal::x86_64::serial) char serial_buf[serial_size];
    hal::interface::serial *my_serial = new((void*)serial_buf) hal::x86_64::serial{*my_port_io};
    my_serial->init_serial(115200);

    constexpr uint16_t print_size = sizeof(kernel::utility::print);
    alignas(kernel::utility::print) char print_buf[print_size];
    kernel::utility::print *my_print = new((void*)print_buf) kernel::utility::print{*my_serial};

    logger::printk("start kernel_main\n");

    my_print->printf("[ INFO ] tick: ");
    while(1)
    {
        // my_print->printf("\e[15G%d\e[22G", timer->get_tick());
        asm volatile ("hlt");
    }

}

