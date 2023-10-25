#include "../hal/x86_64/segment_configurator.hpp"
#include <stdint.h>
#include <cpp_dependent/new.hpp>

#include <interface/interrupt.hpp>
#include "../hal/x86_64/interrupt.hpp"

#include <interface/port_io.hpp>
#include "../hal/x86_64/port_io.hpp"

#include "../hal/x86_64/serial.hpp"

#include "../hal/x86_64/arch_initializer.hpp"

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

void kernel_main(void);

hal::interface::timer *timer;

extern "C" int kernel_entry(boot_info *target_boot_info)
{
    constexpr uint16_t port_io_size = sizeof(hal::x86_64::port_io);
    alignas(hal::x86_64::port_io) char port_io_buf[port_io_size]; 
    hal::interface::port_io *my_port_io = new((void*)port_io_buf) hal::x86_64::port_io{};

    constexpr uint16_t serial_size = sizeof(hal::x86_64::serial);
    alignas(hal::x86_64::serial) char serial_buf[serial_size];
    hal::interface::serial *my_serial = new((void*)serial_buf) hal::x86_64::serial{*my_port_io};
    my_serial->init_serial(115200);

    constexpr uint16_t logger_size = sizeof(kernel::utility::logger);
    alignas(kernel::utility::logger) char logger_buf[logger_size];
    kernel::utility::logger *my_logger = new((void*)logger_buf) kernel::utility::logger{*my_serial};
    using logger = kernel::utility::logger;

    logger::a9nout();
    logger::log("START", "A9N kernel");
    logger::split();

    logger::printk("boot_info->memory_map\n");
    logger::printk("memory_map_max\e[52G:\e[60G%16d\n", target_boot_info->boot_memory_info.memory_map_count);
    logger::printk("memory_map_size\e[52G:\e[60G%16llu B | %6d MiB \n" , target_boot_info->boot_memory_info.memory_size, target_boot_info->boot_memory_info.memory_size / (1024 * 1024));
    logger::printk("memory_map_address\e[52G:\e[58G0x%016x\n", reinterpret_cast<uint64_t>(target_boot_info->boot_memory_info.memory_map));
    logger::split();

    memory_map_entry *target_memory_map_entry = target_boot_info->boot_memory_info.memory_map;

    for (uint16_t i = 0; i < target_boot_info->boot_memory_info.memory_map_count; i++)
    {
        uint64_t memory_map_size = target_memory_map_entry[i].page_count * 4096;
        uint64_t memory_map_size_kb = memory_map_size / 1024;
        uint64_t memory_map_size_mb = memory_map_size_kb / 1024;
        logger::printk("----- memory_map_entry %16d -----\n", i);
        logger::printk("physical_address\e[52G:\e[58G0x%016llx\n", target_memory_map_entry[i].start_physical_address); 
        logger::printk
        (
            "size\e[52G:\e[60G%16d B | %6d KiB | %6d MiB \n",
            memory_map_size,
            memory_map_size_kb,
            memory_map_size_mb
        ); 
        logger::printk("is_free\e[52G:\e[60G%16s\n", target_memory_map_entry[i].is_free ? "yes" : "no"); 
        logger::printk("is_device\e[52G:\e[60G%16s\n", target_memory_map_entry[i].is_device ? "yes" : "no"); 
        logger::split();
    }

    constexpr uint16_t hal_memory_manager_size = sizeof(hal::x86_64::memory_manager);
    alignas(hal::x86_64::memory_manager) char hal_memory_manager_buf[hal_memory_manager_size];
    hal::interface::memory_manager *my_hal_memory_manager = new((void*)hal_memory_manager_buf) hal::x86_64::memory_manager();

    constexpr uint16_t memory_manager_size = sizeof(kernel::memory_manager);
    alignas(kernel::memory_manager) char memory_manager_buf[memory_manager_size];
    kernel::memory_manager *my_memory_manager = new((void*)memory_manager_buf) kernel::memory_manager{*my_hal_memory_manager, target_boot_info->boot_memory_info};
    logger::log("INIT", "memory_manager");
    kernel::physical_address allocate_256kb = my_memory_manager->allocate_physical_memory(2560000, nullptr);
    logger::printk("allocated_address\e[52G:\e[60G0x%016llx\n", reinterpret_cast<uint64_t>(allocate_256kb));
    kernel::physical_address allocate_512kb = my_memory_manager->allocate_physical_memory(5240000, nullptr);
    logger::printk("allocated_address\e[52G:\e[60G0x%016llx\n", reinterpret_cast<uint64_t>(allocate_512kb));
    kernel::physical_address allocate_64b = my_memory_manager->allocate_physical_memory(64, nullptr);
    logger::printk("allocated_address\e[52G:\e[60G0x%016llx\n", reinterpret_cast<uint64_t>(allocate_64b));
    my_memory_manager->deallocate_physical_memory(allocate_256kb, 2560000);
    allocate_64b = my_memory_manager->allocate_physical_memory(64, nullptr);
    logger::printk("allocated_address\e[52G:\e[60G0x%016llx\n", reinterpret_cast<uint64_t>(allocate_64b));
    allocate_64b = my_memory_manager->allocate_physical_memory(64, nullptr);
    logger::printk("allocated_address\e[52G:\e[60G0x%016llx\n", reinterpret_cast<uint64_t>(allocate_64b));
    logger::split();

    my_memory_manager->map_virtual_memory(nullptr, 0xffff800200000000, 0x0000, 3);

    logger::printk("kernel_entry_address\e[52G:\e[60G0x%016llx\n", reinterpret_cast<uint64_t>(kernel_entry));
    logger::log("INIT", "port_io");
    logger::log("INIT", "serial");
    logger::log("INIT", "logger");

    constexpr uint16_t segment_configurator_size = sizeof(hal::x86_64::segment_configurator);
    alignas(hal::x86_64::segment_configurator) char buf[segment_configurator_size];
    hal::x86_64::segment_configurator *my_segment_configurator = new((void*)buf) hal::x86_64::segment_configurator{};
    my_segment_configurator->init_gdt();

    logger::log("INIT", "architecture");

    constexpr uint16_t interrupt_size = sizeof(hal::x86_64::interrupt);
    alignas(hal::x86_64::interrupt) char interrupt_buf[interrupt_size];
    hal::interface::interrupt *my_interrupt = new((void*)interrupt_buf) hal::x86_64::interrupt{};
    my_interrupt->init_interrupt();

    logger::log("INIT", "interrupt");
    logger::log("START", "interrupt");

    constexpr uint16_t timer_size = sizeof(hal::x86_64::pit_timer);
    alignas(hal::x86_64::pit_timer) char timer_buf[timer_size];
    timer = new((void*)timer_buf) hal::x86_64::pit_timer{*my_port_io};

    hal::interface::interrupt_handler timer_interrupt_handler = hal::x86_64::pit_timer::handle;

    my_interrupt->disable_interrupt_all();
    logger::log("DISABLE", "interrupt");

    my_interrupt->register_interrupt(0, timer_interrupt_handler);
    logger::log("REGISTER", "interrupt");

    my_interrupt->enable_interrupt_all();

    logger::log("ENABLE", "interrupt");

    timer->init_timer();
    logger::log("INIT", "timer");
    logger::log("START", "timer");
    logger::log("START", "process_manager");

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

    logger::log("RUN", "kernel_main");

    my_print->printf("[ INFO ] tick: ");
    while(1)
    {
        my_print->printf("\e[15G%d\e[22G", timer->get_tick());
        asm volatile ("hlt");
    }

}

