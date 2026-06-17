#include <kernel/interrupt/interrupt_manager.hpp>

#include <kernel/capability/ipc_port.hpp>
#include <kernel/interrupt/irq_notification_handlers.hpp>
#include <kernel/kernelcall/kernel_call.hpp>
#include <kernel/process/process_manager.hpp>
#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

#include <hal/interface/interrupt.hpp>

namespace a9n::kernel
{
    __attribute__((weak)) extern "C" void *memset(void *buffer, char value, size_t buffer_size)
    {
        uint8_t *buffer_pointer = reinterpret_cast<uint8_t *>(buffer);

        for (size_t i = 0; i < buffer_size; i++)
        {
            *buffer_pointer = value;
            buffer_pointer++;
        }

        return buffer;
    }

    kernel_result interrupt_manager::init(void)
    {
        init_handler();
        init_irq_notification_handlers();

        return {};
    }

    void interrupt_manager::init_handler(void)
    {
        a9n::kernel::utility::logger::printk("Initializing interrupt handlers ...\n");

        a9n::kernel::utility::logger::printk("Registering timer handler ...\n");
        a9n::hal::register_system_timer_handler(handle_timer);

        a9n::kernel::utility::logger::printk("Registering kernel call handler ...\n");
        a9n::hal::register_kernel_call_handler(handle_kernel_call);

        a9n::kernel::utility::logger::printk("Registering interrupt dispatcher ...\n");
        a9n::hal::register_interrupt_dispatcher(handle_interrupt);

        a9n::kernel::utility::logger::printk("Registering fault dispatcher ...\n");
        a9n::hal::register_fault_dispatcher(handle_fault);
    }

    void interrupt_manager::init_irq_notification_handlers(void)
    {
        a9n::kernel::utility::logger::printk("Initializing IRQ notification handlers ...\n");

        for (auto i = 0; i < a9n::hal::IRQ_NUMBER_MAX; i++)
        {
            irq_notification_handlers[i].irq_number = i;
            irq_notification_handlers[i].slot.init();
        }
    }

    kernel_result interrupt_manager::enable_interrupt(a9n::word irq_number)
    {
        DEBUG_LOG("Enabling interrupt [ %llu ] ...\n", irq_number);
        return a9n::hal::enable_interrupt(irq_number).transform_error(convert_hal_to_kernel_error);
    }

    kernel_result interrupt_manager::disable_interrupt(a9n::word irq_number)
    {
        DEBUG_LOG("Disabling interrupt [ %llu ] ...\n", irq_number);
        return a9n::hal::disable_interrupt(irq_number).transform_error(convert_hal_to_kernel_error);
    }

    kernel_result interrupt_manager::enable_interrupt_all(void)
    {
        a9n::kernel::utility::logger::printk("Enabling all interrupts ...\n");
        return a9n::hal::enable_interrupt_all().transform_error(convert_hal_to_kernel_error);
    }

    kernel_result interrupt_manager::disable_interrupt_all(void)
    {
        a9n::kernel::utility::logger::printk("Disabling all interrupts ...\n");
        return a9n::hal::disable_interrupt_all().transform_error(convert_hal_to_kernel_error);
    }

    kernel_result interrupt_manager::ack_interrupt(void)
    {
        return a9n::hal::ack_interrupt().transform_error(convert_hal_to_kernel_error);
    }

    // this handler is required
    extern "C" void handle_timer(void)
    {
        auto result = a9n::kernel::process_manager_core.handle_timer().and_then(
            [](void) -> kernel_result
            {
                a9n::kernel::interrupt_manager_core.ack_interrupt();
                return {};
            }
        );
    }

    extern "C" void handle_interrupt(a9n::word irq_number)
    {
        irq_number_to_irq_notification_handler(irq_number)
            .and_then(
                [](liba9n::not_null<irq_notification_handler> handler) -> kernel_result
                {
                    if (handler->slot.type == capability_type::NONE)
                    {
                        DEBUG_LOG("No handler for IRQ number %04llu", handler->irq_number);
                        return kernel_error::TRY_AGAIN;
                    }

                    if ((handler->slot.type != capability_type::NOTIFICATION_PORT)
                        || !(handler->slot.component)) [[unlikely]]
                    {
                        DEBUG_LOG("Invalid handler type for IRQ_number %llu", handler->irq_number);
                        return kernel_error::TRY_AGAIN;
                    }

                    return process_manager_core.retrieve_current_process().and_then(
                        [&handler](process *current_process) -> kernel_result
                        {
                            auto &port
                                = reinterpret_cast<notification_port &>(*(handler->slot.component));

                            return port.operation_notify(*current_process, handler->slot)
                                .transform_error(
                                    [&handler](capability_error e) -> kernel_error
                                    {
                                        return kernel_error::TRY_AGAIN;
                                    }
                                )
                                .and_then(
                                    [](void) -> kernel_result
                                    {
                                        return a9n::kernel::interrupt_manager_core.ack_interrupt();
                                    }
                                )
                                .and_then(
                                    [&](void) -> kernel_result
                                    {
                                        return interrupt_manager_core.disable_interrupt(
                                            handler->irq_number
                                        );
                                    }
                                )
                                .and_then(
                                    [&](void) -> kernel_result
                                    {
                                        return {};

                                        // WARN: NOT WORKING
                                        /*
                                        return a9n::kernel::process_manager_core
                                            .reschedule_from_interrupt();
                                        */
                                    }
                                );
                        }
                    );
                }
            )
            .or_else(
                [irq_number](kernel_error e) -> kernel_result
                {
                    DEBUG_LOG("error : %s", a9n::kernel::kernel_error_to_string(e));
                    return e;
                }
            );
    }

    extern "C" void handle_fault(
        a9n::kernel::fault_type type,
        a9n::sword              fault_code, // type-specific code
        a9n::word               arch_fault_code,
        a9n::virtual_address    fault_address
    )
    {
        DEBUG_LOG(
            "Handle fault : type = %s, fault_code = %d, arch_fault_code = %d, fault_address = "
            "0x%016llx",
            a9n::kernel::fault_type_to_string(type),
            fault_code,
            arch_fault_code,
            fault_address
        );
        process_manager_core.retrieve_current_process()
            .transform_error(convert_kernel_to_capability_error)
            .and_then(
                [type, fault_code, arch_fault_code, fault_address](process *current_process) -> capability_result
                {
                    current_process->fault_reason    = type;
                    current_process->fault_address   = fault_address;
                    current_process->fault_code      = fault_code;
                    current_process->arch_fault_code = arch_fault_code;

                    if ((current_process->resolver_port.type != capability_type::IPC_PORT)
                        || !current_process->resolver_port.component) [[unlikely]]
                    {
#ifdef A9N_CONFIG_ENABLE_LOG_DOUBLE_FAULT
                        kernel::utility::logger::printk(
                            "Double fault : %s, fault_code : %lld, arch_fault_code: %llu, "
                            "fault_address : 0x%016llx\n",
                            a9n::kernel::fault_type_to_string(type),
                            fault_code,
                            arch_fault_code,
                            fault_address
                        );
#endif
                        // [double fault]
                        // NOTE: at the time of fault handler, current process is still the process
                        // where the fault occurred; therefore, it is necessary to execute
                        // re-schedule + switch even if resolver does not exist
                        current_process->status = process_status::BLOCKED_SUSPEND;
                        return capability_error::INVALID_DESCRIPTOR;
                    }

                    ipc_port &port = static_cast<ipc_port &>(*current_process->resolver_port.component);
                    return port
                        .operation_fault_call(*current_process, current_process->resolver_port)
                        .or_else(
                            [current_process](capability_error e) -> capability_result
                            {
                                current_process->status = process_status::BLOCKED_SUSPEND;
                                return e;
                            }
                        );
                }
            )
            .or_else(
                [=](capability_error e) -> kernel_result
                {
#ifdef A9N_CONFIG_ENABLE_LOG_FAULT
                    a9n::kernel::utility::logger::printk(
                        "Fault : %s, fault_code : %lld, arch_fault_code: %llu, fault_address : "
                        "0x%016llx\n",
                        a9n::kernel::fault_type_to_string(type),
                        fault_code,
                        arch_fault_code,
                        fault_address
                    );
#endif

                    return process_manager_core.try_schedule_and_switch();
                }
            );
    }
}
