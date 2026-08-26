#include <kernel/types.hpp>
#include <stdint.h>

#include <kernel/boot/boot_info.hpp>
#include <kernel/boot/init.hpp>
#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_node.hpp>
#include <kernel/capability/generic.hpp>
#include <kernel/interrupt/interrupt_manager.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/process/cpu.hpp>
#include <kernel/process/process.hpp>
#include <kernel/process/process_manager.hpp>
#include <kernel/time/timer.hpp>
#include <kernel/utility/logger.hpp>
#include <kernel/version.hpp>

#include <hal/interface/hal.hpp>
#include <hal/interface/hal_factory.hpp>
#include <hal/interface/interrupt.hpp>
#include <hal/interface/memory_manager.hpp>
#include <hal/interface/port_io.hpp>
#include <hal/interface/timer.hpp>

// TODO: devirtualize and remove hal_factory
#include <hal/x86_64/factory/hal_factory.hpp>

#include <liba9n/common/allocator.hpp>
#include <liba9n/common/calculate.hpp>
#include <liba9n/libc/string.hpp>
#include <liba9n/libcxx/new>
#include <liba9n/option/option.hpp>
#include <liba9n/result/result.hpp>

void kernel_main(void);

a9n::hal::hal *hal_instance;

constexpr uint32_t hal_factory_size = sizeof(a9n::hal::x86_64::hal_factory);
alignas(a9n::hal::x86_64::hal_factory) static char hal_factory_buffer[hal_factory_size];

extern "C" int kernel_entry(a9n::kernel::boot_info *target_boot_info)
{
    a9n::kernel::kernel_result result = {};

    using logger                      = a9n::kernel::utility::logger;
    // make HAL and kernel objects.
    a9n::hal::hal_factory *hal_factory_instance = new (hal_factory_buffer)
        a9n::hal::x86_64::hal_factory();
    hal_instance = hal_factory_instance->make();

    hal_instance->_serial->init_serial(115200);

    constexpr uint16_t                         logger_size = sizeof(a9n::kernel::utility::logger);
    alignas(a9n::kernel::utility::logger) char logger_buf[logger_size];
    a9n::kernel::utility::logger              *my_logger = new ((void *)logger_buf)
        a9n::kernel::utility::logger { *hal_instance->_serial };

    // reset terminal and print boot information
    logger::printn("\e[0m\e[2J\e[H");
    logger::printk("Booting the A9N Microkernel ...\n");
    logger::a9nout();
    logger::printk("Kernel entry: address=%p\n", kernel_entry);
    logger::printk("Boot information: address=%p\n", reinterpret_cast<uint64_t>(target_boot_info));

    // init cpu local variables
    logger::printk("Initializing per-CPU local variables ...\n");
    a9n::kernel::init_cpu_local_variable();

    logger::printk("Initializing architecture (HAL) ...\n");
    auto arch_res = hal_instance->_arch_initializer->init_architecture(target_boot_info->arch_info);
    if (!arch_res)
    {
        logger::error("Failed to initialize architecture!");
        return 0;
    }

    logger::printk("Initializing interrupt system ...\n");
    result = a9n::kernel::interrupt_manager_core.init();
    a9n::kernel::interrupt_manager_core.ack_interrupt();

    logger::printk(
        "Configuring system clock frequency: %llu Hz ...\n",
        static_cast<a9n::word>(a9n::kernel::SYSTEM_CLOCK_FREQUENCY)
    );
    auto clock_result = a9n::hal::configure_system_clock_frequency(a9n::kernel::SYSTEM_CLOCK_FREQUENCY);
    if (!clock_result)
    {
        logger::error("Failed to configure system clock frequency");
        return 0;
    }

    logger::printk("Initializing process-management system ...\n");
    result = a9n::kernel::init_idle_context().and_then(
        [](void) -> a9n::kernel::kernel_result
        {
            return a9n::kernel::cpu_local_variables[a9n::kernel::BSP_ID].process_manager_core.init(
                a9n::kernel::BSP_ID
            );
        }
    );

    // create user thread
    logger::printk("Initializing init process ...\n");
    auto init_res = create_init(*target_boot_info);
    if (!init_res)
    {
        logger::error("Failed to create init!");
        for (;;)
            ;
    }

    logger::printk("All initializations completed successfully, Launching init ...\n");
    auto start_other_cores_result = a9n::hal::start_other_cores();
    if (!start_other_cores_result)
    {
        logger::error("Failed to start application processors");
        return 0;
    }
    a9n::kernel::cpu_local_variables[a9n::kernel::BSP_ID].process_manager_core.switch_to_user();

    for (;;)
        ;

    return 2038;
}

extern "C" void ap_main(void)
{
}
