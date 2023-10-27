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
#include "process_manager.hpp"

void kernel_main(void);


hal::interface::hal *hal_instance;

constexpr uint32_t hal_factory_size = sizeof(hal::x86_64::hal_factory);
alignas(hal::x86_64::hal_factory) static char hal_factory_buffer[hal_factory_size];

static uint32_t clock_count = 0;

__attribute__((interrupt))
extern "C" void handle_timer(void *data)
{
    if (clock_count >= 10)
    {
        kernel::utility::logger::printk("context_switch\n");
        clock_count = 0;
        kernel::kernel_object::process_manager->switch_context();
    }
    clock_count++;
    // kernel::utility::logger::printk("clock_count : %d\n", clock_count);
    hal_instance->_timer->clock();
}

__attribute__((interrupt))
void handle_page_fault(void *data, uint64_t error_code)
{
    kernel::utility::logger::printk("page_fault : %llx\n", error_code);
    uint64_t cr2_value;
    asm volatile
    (
        "mov %%cr2, %0;"
        : "=r"(cr2_value)
        :
        :
    );
    kernel::utility::logger::printk("CR2 : 0x%016llx\n", cr2_value);
}

__attribute__((interrupt))
void exception_handler(void *data, uint64_t error_code)
{
    kernel::utility::logger::printk("exception\n", error_code);
}

void process_1()
{
    while(true)
    {
        for (uint32_t i = 0; i < 50000000; i++)
        {
            asm volatile ("nop");
        }

        kernel::utility::logger::printk("process_1\n");
        kernel::kernel_object::process_manager->switch_context();
    }
}

void process_2()
{
    while(true)
    {
        for (uint32_t i = 0; i < 50000000; i++)
        {
            asm volatile ("nop");
        }

        kernel::utility::logger::printk("process_2\n");
        kernel::kernel_object::process_manager->switch_context();
    }
}

void process_3()
{
    while(true)
    {
        for (uint32_t i = 0; i < 50000000; i++)
        {
            asm volatile ("nop");
        }

        kernel::utility::logger::printk("process_3\n");
        kernel::kernel_object::process_manager->switch_context();
    }
}

void process_4()
{
    while(true)
    {
        for (uint32_t i = 0; i < 50000000; i++)
        {
            asm volatile ("nop");
        }

        kernel::utility::logger::printk("process_4\n");
        kernel::kernel_object::process_manager->switch_context();
    }
}

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
    logger::printk("boot_info_address\e[52G:\e[60G0x%016llx\n", reinterpret_cast<uint64_t>(target_boot_info));
    logger::split();

    logger::printk("init hal\n");
    logger::printk("init port_io\n");
    logger::printk("init serial\n");
    logger::printk("init logger\n");

    logger::printk("create memory_manager\n");
    kernel::kernel_object::memory_manager = new(kernel::kernel_object::memory_manager_buffer) kernel::memory_manager(*hal_instance->_memory_manager, target_boot_info->boot_memory_info);

    logger::printk("test memory_manager\n");
    kernel::kernel_object::memory_manager->allocate_physical_memory(40, nullptr);
    kernel::kernel_object::memory_manager->map_virtual_memory(nullptr, 0xffff800200000000, 0x0000, 3);

    logger::printk("init architecture\n");
    hal_instance->_arch_initializer->init_architecture();

    logger::printk("init interrupt\n");
    hal_instance->_interrupt->init_interrupt();
    
    hal_instance->_interrupt->disable_interrupt_all();

    // hal_instance->_interrupt->register_interrupt(0, (hal::interface::interrupt_handler) handle_timer);
    // hal_instance->_timer->init_timer();
    // hal_instance->_interrupt->register_interrupt(14, (hal::interface::interrupt_handler) handle_page_fault);

    // test process_manager
    logger::printk("init process_manager\n");
    kernel::kernel_object::process_manager = new(kernel::kernel_object::process_manager_buffer) kernel::process_manager(*hal_instance->_process_manager);
    kernel::kernel_object::process_manager->create_process("process_1", reinterpret_cast<kernel::virtual_address>(process_1));
    kernel::kernel_object::process_manager->create_process("process_2", reinterpret_cast<kernel::virtual_address>(process_2));
    kernel::kernel_object::process_manager->create_process("process_3", reinterpret_cast<kernel::virtual_address>(process_3));
    kernel::kernel_object::process_manager->create_process("process_4", reinterpret_cast<kernel::virtual_address>(process_4));

    hal_instance->_interrupt->enable_interrupt_all();
    kernel_main();

    return 2038;
}

void kernel_main(void)
{
    kernel::utility::logger::printk("start kernel_main\n");
    process_1();

    while(1)
    {
        // my_print->printf("\e[15G%d\e[22G", timer->get_tick());
        // kernel::utility::logger::printk("hlt\n");
        asm volatile ("hlt");
    }

}
