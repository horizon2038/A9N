#include "hal/hal_result.hpp"
#include "kernel/capability/capability_types.hpp"
#include "kernel/memory/memory_type.hpp"
#include <liba9n/libcxx/new>
#include <stdint.h>

#include <hal/interface/hal.hpp>
#include <hal/interface/hal_factory.hpp>
#include <hal/interface/interrupt.hpp>
#include <hal/interface/memory_manager.hpp>
#include <hal/interface/port_io.hpp>
#include <hal/interface/timer.hpp>

#include <kernel/boot/boot_info.hpp>
#include <kernel/interrupt/interrupt_manager.hpp>
// #include <kernel/ipc/ipc_manager.hpp>
#include <kernel/memory/memory_manager.hpp>
#include <kernel/process/cpu.hpp>
#include <kernel/process/process.hpp>
#include <kernel/process/process_manager.hpp>
#include <kernel/utility/logger.hpp>

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_node.hpp>
#include <kernel/capability/generic.hpp>

#include <kernel/kernel_result.hpp>
#include <kernel/types.hpp>

#include <liba9n/libc/string.hpp>

#include <hal/x86_64/factory/hal_factory.hpp>

#include <liba9n/common/allocator.hpp>
#include <liba9n/common/calculate.hpp>
#include <liba9n/option/option.hpp>
#include <liba9n/result/result.hpp>

#include <kernel/boot/init.hpp>

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

    // init cpu local variables
    a9n::kernel::init_cpu_local_variable();

    // init cpu local variables
    a9n::hal::configure_local_variable(&a9n::kernel::cpu_local_variables[0]);
    logger::printk("init architecture\n");
    logger::printk(
        "arch_initializer\e[52G:\e[60G0x%016llx\n",
        reinterpret_cast<uint64_t>(hal_instance->_arch_initializer)
    );

    auto arch_res = hal_instance->_arch_initializer->init_architecture(target_boot_info->arch_info);
    if (!arch_res)
    {
        logger::error("failed to init architecture");
        return 0;
    }

    logger::printk("init hal\n");
    logger::printk("init io\n");
    logger::printk("init serial\n");
    logger::printk("init logger\n");

    logger::printk("init interrupt_manager\n");
    result = a9n::kernel::interrupt_manager_core.init();

    logger::printk("disable_interrupt\n");
    a9n::kernel::interrupt_manager_core.disable_interrupt_all();

    logger::printk("init process_manager\n");
    result = a9n::kernel::process_manager_core.init();
    logger::split();

    // create user thread
    logger::printk("create initial threads ...\n");

    auto init_res = create_init(*target_boot_info);
    if (!init_res)
    {
        logger::error("failed to create init");
        for (;;)
            ;
    }

    logger::printk("all initialization completed successfully\n");

    // a9n::kernel::interrupt_manager_core.ack_interrupt();
    // a9n::kernel::interrupt_manager_core.enable_interrupt_all();

    a9n::kernel::process_manager_core.switch_to_user();

    for (;;)
        ;

    return 2038;
}
