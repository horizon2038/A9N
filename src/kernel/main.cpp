#include <stdint.h>
#include <library/cpp_dependent/new.hpp>

#include <hal/interface/interrupt.hpp>
#include <hal/interface/port_io.hpp>
#include <hal/interface/memory_manager.hpp>
#include <hal/interface/hal.hpp>
#include <hal/interface/hal_factory.hpp>
#include <hal/interface/timer.hpp>

#include <kernel/utility/logger.hpp>

#include <library/common/types.hpp>
#include <kernel/interrupt/interrupt_manager.hpp>
#include <kernel/ipc/ipc_manager.hpp>
#include <kernel/process/process.hpp>

#include <kernel/boot/boot_info.hpp>

#include <kernel/memory/memory_manager.hpp>

#include <kernel/kernel.hpp>
#include <kernel/process/process_manager.hpp>

#include <library/libc/string.hpp>

#include <hal/x86_64/factory/hal_factory.hpp>

void kernel_main(void);

hal::interface::hal *hal_instance;

constexpr uint32_t hal_factory_size = sizeof(hal::x86_64::hal_factory);
alignas(hal::x86_64::hal_factory
) static char hal_factory_buffer[hal_factory_size];

void read_serial()
{
    while (1)
    {
        asm volatile("sti");
        library::ipc::message m;
        m.type = 1;
        uint8_t serial_data = hal_instance->_serial->read_serial();
        if (serial_data == 0xd)
        {
            std::strcpy(reinterpret_cast<char *>(m.data), "\r\n");
            kernel::kernel_object::ipc_manager->send(4, &m);
            hal_instance->_interrupt->ack_interrupt();
            continue;
        }
        if (serial_data == 0x7f)
        {
            std::strcpy(reinterpret_cast<char *>(m.data), "\b\033[K");
            kernel::kernel_object::ipc_manager->send(4, &m);
            hal_instance->_interrupt->ack_interrupt();
            continue;
        }
        char c[2];
        c[0] = serial_data;
        c[1] = '\0';
        std::strcpy(reinterpret_cast<char *>(m.data), c);
        kernel::kernel_object::ipc_manager->send(4, &m);
        hal_instance->_interrupt->ack_interrupt();
    }
}

void console()
{
    char buffer[256];
    uint32_t buffer_index = 0;
    std::memset(buffer, 0, sizeof(buffer));

    kernel::utility::logger::printn("\e[32mhorizon@A9N\e[0m > ");

    while (1)
    {
        asm volatile("sti");
        library::ipc::message m;
        kernel::kernel_object::ipc_manager->receive(kernel::ANY_PROCESS, &m);

        // Assuming that m.data holds ASCII characters
        char received_char = reinterpret_cast<char *>(m.data)[0];

        if (received_char == '\0') // no character received
        {
            continue;
        }
        else if (received_char == '\r') // Enter key
        {
            buffer[buffer_index] = '\0'; // null terminate the string

            // Prepare a message to send buffer content to process 3
            library::ipc::message buffer_message;
            buffer_message.type = 1; // assuming 1 is a generic message type

            // Manually copy buffer content to buffer_message.data
            for (uint32_t i = 0; i <= buffer_index; ++i)
            {
                buffer_message.data[i] = buffer[i];
            }

            kernel::kernel_object::ipc_manager->send(
                5,
                &buffer_message
            ); // send buffer content to process 3
            buffer_index = 0; // reset buffer index
            std::memset(buffer, 0, sizeof(buffer)); // clear the buffer
            kernel::utility::logger::printn("\e[32mhorizon@A9N\e[0m > ");
        }

        else if (received_char == '\b') // Backspace key
        {
            if (buffer_index > 0)
            {
                buffer[--buffer_index]
                    = '\0'; // remove the last character from buffer
                kernel::utility::logger::printn("\b \b"
                ); // handle backspace on terminal
            }
        }
        else if (buffer_index < sizeof(buffer) - 1) // buffer not full
        {
            buffer[buffer_index++]
                = received_char; // store the character in buffer
            kernel::utility::logger::printn(
                "%c",
                received_char
            ); // print the received character
        }
    }
}

void console_out()
{
    while (1)
    {
        asm volatile("sti");
        library::ipc::message m;
        // Assuming process ID 3 is for console_out
        kernel::kernel_object::ipc_manager->receive(kernel::ANY_PROCESS, &m);

        char *received_data = reinterpret_cast<char *>(m.data);

        if (m.type != 1)
        {
            continue;
        }
        kernel::utility::logger::printn("\ncout said : %s\n", received_data);

        if (std::strcmp(received_data, "about") == 0)
        {
            kernel::utility::logger::a9nout();
        }

        if (std::strcmp(received_data, "syscall") == 0)
        {
            asm volatile("int $0x80" ::: "cc", "memory");
        }

        if (std::strcmp(received_data, "info pm") == 0)
        {
            library::ipc::message m2;
            m2.type = 1;
            // Assuming process ID 3 is for console_out
            std::strcpy(reinterpret_cast<char *>(m2.data), "info pm");
            kernel::kernel_object::ipc_manager->send(2, &m2);
        }
        if (std::strcmp(received_data, "mitoujr") == 0)
        {
            kernel::utility::logger::mitoujr();
        }
    }
}

void info_mem()
{
    while (1)
    {
        asm volatile("sti");
        library::ipc::message m;
        kernel::kernel_object::ipc_manager->receive(3, &m);
        if (m.type != 1)
        {
            continue;
        }
        kernel::kernel_object::memory_manager->info_physical_memory();
    }
}

void alpha()
{
    asm volatile("sti");
    kernel::utility::logger::printn("alpha\n");
    kernel::kernel_object::process_manager->create_process(
        "read_serial",
        reinterpret_cast<kernel::virtual_address>(read_serial)
    );
    kernel::kernel_object::process_manager->create_process(
        "console",
        reinterpret_cast<kernel::virtual_address>(console)
    );
    kernel::kernel_object::process_manager->create_process(
        "console_out",
        reinterpret_cast<kernel::virtual_address>(console_out)
    );
    // read_serial : 3
    // console : 4
    // console_out : 5
    while (true)
    {
        asm volatile("sti");
        library::ipc::message m;
        kernel::kernel_object::ipc_manager->receive(kernel::ANY_PROCESS, &m);
        if (m.type != 1)
        {
            continue;
        }
        char *received_data = reinterpret_cast<char *>(m.data);
        kernel::utility::logger::printk("received : %s\n", received_data);
        if (std::strcmp(received_data, "info pm") == 0)
        {
            kernel::kernel_object::memory_manager->info_physical_memory();
        }
    }
}

extern "C" int kernel_entry(boot_info *target_boot_info)
{
    using logger = kernel::utility::logger;
    // make HAL and kernel objects.
    hal::interface::hal_factory *hal_factory_instance
        = new (hal_factory_buffer) hal::x86_64::hal_factory();
    hal_instance = hal_factory_instance->make();

    hal_instance->_serial->init_serial(115200);

    constexpr uint16_t logger_size = sizeof(kernel::utility::logger);
    alignas(kernel::utility::logger) char logger_buf[logger_size];
    kernel::utility::logger *my_logger = new ((void *)logger_buf)
        kernel::utility::logger { *hal_instance->_serial };

    logger::a9nout();
    logger::printk("start A9N kernel\n");
    logger::split();

    logger::printk(
        "kernel_entry_address\e[52G:\e[60G0x%016llx\n",
        reinterpret_cast<uint64_t>(kernel_entry)
    );
    logger::printk(
        "boot_info_address\e[52G:\e[60G0x%016llx\n",
        reinterpret_cast<uint64_t>(target_boot_info)
    );
    logger::split();

    logger::printk("init hal\n");
    logger::printk("init io\n");
    logger::printk("init serial\n");
    logger::printk("init logger\n");

    logger::printk("init memory_manager\n");
    kernel::kernel_object::memory_manager
        = new (kernel::kernel_object::memory_manager_buffer)
            kernel::memory_manager(
                *hal_instance->_memory_manager,
                target_boot_info->boot_memory_info
            );

    logger::printk("test memory_manager\n");
    kernel::kernel_object::memory_manager->allocate_physical_memory(
        40,
        nullptr
    );
    kernel::kernel_object::memory_manager
        ->map_virtual_memory(nullptr, 0xffff800200000000, 0x0000, 3);

    logger::printk("init interrupt_manager\n");
    kernel::kernel_object::interrupt_manager
        = new (kernel::kernel_object::interrupt_manager_buffer)
            kernel::interrupt_manager(*hal_instance->_interrupt);
    kernel::kernel_object::interrupt_manager->init();

    logger::printk("init ipc_manager\n");
    kernel::kernel_object::ipc_manager
        = new (kernel::kernel_object::ipc_manager_buffer) kernel::ipc_manager();

    kernel::kernel_object::interrupt_manager->disable_interrupt_all();

    logger::printk("init architecture\n");
    hal_instance->_arch_initializer->init_architecture();

    hal_instance->_timer->init_timer();

    // test process_manager
    logger::printk("init process_manager\n");
    kernel::kernel_object::process_manager
        = new (kernel::kernel_object::process_manager_buffer)
            kernel::process_manager(*hal_instance->_process_manager);
    logger::split();
    // 最終成果報告会用バックアップ
    /*
    kernel::kernel_object::process_manager->create_process("read_serial",
    reinterpret_cast<kernel::virtual_address>(read_serial));
    kernel::kernel_object::process_manager->create_process("console",
    reinterpret_cast<kernel::virtual_address>(console));
    kernel::kernel_object::process_manager->create_process("console_out",
    reinterpret_cast<kernel::virtual_address>(console_out));
    kernel::kernel_object::process_manager->create_process("info pm",
    reinterpret_cast<kernel::virtual_address>(info_mem));
    kernel::kernel_object::process_manager->create_process("idle",
    reinterpret_cast<kernel::virtual_address>(kernel_main));
    */

    kernel::kernel_object::process_manager->create_process(
        "idle",
        reinterpret_cast<kernel::virtual_address>(kernel_main)
    );
    kernel::kernel_object::process_manager->create_process(
        "alpha",
        reinterpret_cast<kernel::virtual_address>(alpha)
    );

    kernel::kernel_object::interrupt_manager->enable_interrupt_all();
    kernel::kernel_object::interrupt_manager->ack_interrupt();

    kernel_main();

    return 2038;
}

void kernel_main(void)
{
    while (1)
    {
        // kernel::utility::logger::printk("IDLE \n");
        // kernel::kernel_object::interrupt_manager->ack_interrupt();
        asm volatile("sti");
        kernel::kernel_object::process_manager->switch_context();
        asm volatile("hlt");
    }
}
