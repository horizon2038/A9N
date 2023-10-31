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
#include "interrupt_manager.hpp"
#include "ipc_manager.hpp"
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

#include <library/string.hpp>

void kernel_main(void);


hal::interface::hal *hal_instance;

constexpr uint32_t hal_factory_size = sizeof(hal::x86_64::hal_factory);
alignas(hal::x86_64::hal_factory) static char hal_factory_buffer[hal_factory_size];

__attribute__((interrupt)) void handle_timer(void *data)
{
    hal_instance->_interrupt->ack_interrupt();
    kernel::kernel_object::process_manager->switch_context();
}

__attribute__((interrupt))
extern "C" void handle_serial(void *data)
{
    uint8_t serial_data = hal_instance->_serial->read_serial();
    if (serial_data == 13)
    {
        serial_data = '\n';
    }
    kernel::utility::logger::printk("%x", serial_data);
    hal_instance->_interrupt->ack_interrupt();
}

void process_1()
{
    while (true)
    {
        asm volatile ("sti");
        for (uint32_t i = 0; i < 100000000; i++)
        {
            asm volatile ("nop");
        }
        message m;
        m.type = 1;
        std::strcpy(reinterpret_cast<char*>(m.data), "hello from process_1");
        kernel::kernel_object::ipc_manager->send(2, &m);
        kernel::utility::logger::printk("process_1\n");
        kernel::utility::logger::split();
    }
}

void process_2()
{
    while (true)
    {
        asm volatile ("sti");
        for (uint32_t i = 0; i < 100000000; i++)
        {
            asm volatile ("nop");
        }
        message m;
        kernel::kernel_object::ipc_manager->receive(kernel::ANY_PROCESS, &m);
        kernel::utility::logger::printk("process_2\n");
        kernel::utility::logger::printk("receive : %s\n", reinterpret_cast<char*>(m.data));
        kernel::utility::logger::split();
    }
}

void process_3()
{
    while(true)
    {
        asm volatile ("sti");
        for (uint32_t i = 0; i < 1000000000; i++)
        {
            asm volatile ("nop");
        }
        message m;
        m.type = 1;
        std::strcpy(reinterpret_cast<char*>(m.data), "hello from process_3");
        kernel::kernel_object::ipc_manager->send(2, &m);
        kernel::utility::logger::printk("process_3\n");
        kernel::utility::logger::split();
    }
}

void process_4()
{
    while(true)
    {
        asm volatile ("sti");
        for (uint32_t i = 0; i < 1000000000; i++)
        {
            asm volatile ("nop");
        }
        message m;
        m.type = 1;
        std::strcpy(reinterpret_cast<char*>(m.data), "hello from process_4");
        kernel::kernel_object::ipc_manager->send(2, &m);
        kernel::utility::logger::printk("process_4\n");
        kernel::utility::logger::split();
    }
}

void read_serial()
{
    while(1)
    {
        asm volatile ("sti");
        for (uint32_t i = 0; i < 1000; i++)
        {
            asm volatile ("nop");
        }
        message m;
        m.type = 1;
        uint8_t serial_data = hal_instance->_serial->read_serial();
        if (serial_data == 0xd)
        {
            std::strcpy(reinterpret_cast<char*>(m.data), "\r\nhorizon@A9N >");
            kernel::kernel_object::ipc_manager->send(2, &m);
            hal_instance->_interrupt->ack_interrupt();
            continue;
        }
        if (serial_data == 0x7f)
        {
            std::strcpy(reinterpret_cast<char*>(m.data), "\b\033[K");
            kernel::kernel_object::ipc_manager->send(2, &m);
            hal_instance->_interrupt->ack_interrupt();
            continue;
        }
        char c[2];
        c[0] = serial_data;
        c[1] = '\0';
        std::strcpy(reinterpret_cast<char*>(m.data), c);
        kernel::kernel_object::ipc_manager->send(2, &m);
        hal_instance->_interrupt->ack_interrupt();
    }
}

void console()
{
    while(1)
    {
        asm volatile ("sti");
        for (uint32_t i = 0; i < 1000; i++)
        {
            asm volatile ("nop");
        }
        message m;
        kernel::kernel_object::ipc_manager->receive(kernel::ANY_PROCESS, &m);
        if ((char*)m.data[0] == 0)
        {
            continue;
        }
        kernel::utility::logger::printk("horizon@A9N > " "%s\n", reinterpret_cast<char*>(m.data));
    }
}

extern "C" int kernel_entry(boot_info *target_boot_info)
{
    using logger = kernel::utility::logger;
    // make HAL and kernel objects.
    hal::interface::hal_factory *hal_factory_instance = new (hal_factory_buffer) hal::x86_64::hal_factory();
    hal_instance = hal_factory_instance->make();

    hal_instance->_serial->init_serial(115200);

    constexpr uint16_t logger_size = sizeof(kernel::utility::logger);
    alignas(kernel::utility::logger) char logger_buf[logger_size];
    kernel::utility::logger *my_logger = new((void*)logger_buf) kernel::utility::logger{*hal_instance->_serial};

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

    logger::printk("create interrupt_manager\n");
    kernel::kernel_object::interrupt_manager = new(kernel::kernel_object::interrupt_manager_buffer) kernel::interrupt_manager(*hal_instance->_interrupt);


    logger::printk("init interrupt\n");
    kernel::kernel_object::interrupt_manager->init();
    
    hal_instance->_interrupt->disable_interrupt_all();

    logger::printk("init architecture\n");
    hal_instance->_arch_initializer->init_architecture();

    // hal_instance->_interrupt->register_interrupt(0, (hal::interface::interrupt_handler) handle_timer);
    hal_instance->_timer->init_timer();

    // test process_manager
    logger::printk("init process_manager\n");
    kernel::kernel_object::process_manager = new(kernel::kernel_object::process_manager_buffer) kernel::process_manager(*hal_instance->_process_manager);
    // kernel::kernel_object::process_manager->create_process("process_1", reinterpret_cast<kernel::virtual_address>(process_1));
    // kernel::kernel_object::process_manager->create_process("process_2", reinterpret_cast<kernel::virtual_address>(process_2));
    // kernel::kernel_object::process_manager->create_process("process_3", reinterpret_cast<kernel::virtual_address>(process_3));
    // kernel::kernel_object::process_manager->create_process("process_4", reinterpret_cast<kernel::virtual_address>(process_4));
    kernel::kernel_object::process_manager->create_process("read_serial", reinterpret_cast<kernel::virtual_address>(read_serial));
    kernel::kernel_object::process_manager->create_process("console", reinterpret_cast<kernel::virtual_address>(console));

    hal_instance->_interrupt->enable_interrupt_all();

    // process_1();
    
    kernel_main();

    return 2038;
}

void kernel_main(void)
{
    kernel::utility::logger::printk("start kernel_main\n");
    kernel::utility::logger::printk("horizon@A9N > ");
    // process_1();

    while(1)
    {
        for (uint32_t i = 0; i < 1000; i++)
        {
            asm volatile ("nop");
        }
        uint8_t serial_data = hal_instance->_serial->read_serial();
        if (serial_data == 0xd)
        {
            hal_instance->_serial->write_serial('\r');
            hal_instance->_serial->write_serial('\n');
            kernel::utility::logger::printk("");
            hal_instance->_serial->write_string_serial("horizon@A9N > ");
            hal_instance->_interrupt->ack_interrupt();
            continue;
        }
        if (serial_data == 0x7f)
        {
            hal_instance->_serial->write_serial('\b');
            hal_instance->_serial->write_string_serial("\033[K");
            hal_instance->_interrupt->ack_interrupt();
            continue;
        }
        hal_instance->_serial->write_serial(serial_data);
        hal_instance->_interrupt->ack_interrupt();
        asm volatile ("hlt");
    }

}
