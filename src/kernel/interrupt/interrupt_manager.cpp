#include <kernel/interrupt/interrupt_manager.hpp>

#include <kernel/capability/ipc_port.hpp>
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

        return {};
    }

    void interrupt_manager::init_handler(void)
    {
        a9n::kernel::utility::logger::printk("init interrupt handlers ...\n");
        a9n::hal::register_system_timer_handler(handle_timer);
        a9n::hal::register_kernel_call_handler(handle_kernel_call);
        a9n::hal::register_interrupt_dispatcher(handle_interrupt);
        a9n::hal::register_fault_dispatcher(handle_fault);
    }

    void interrupt_manager::enable_interrupt_all(void)
    {
        a9n::kernel::utility::logger::printk("enable interrupt ...\n");
        a9n::hal::enable_interrupt_all();
    }

    void interrupt_manager::disable_interrupt_all(void)
    {
        a9n::kernel::utility::logger::printk("disable interrupt ...\n");
        a9n::hal::disable_interrupt_all();
    }

    void interrupt_manager::ack_interrupt(void)
    {
        a9n::hal::ack_interrupt();
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
        // TODO: implement notification
        a9n::kernel::utility::logger::printk("hello from handle_interrupt [ 0x%4llu ]\n", irq_number);
    }

    extern "C" void handle_fault(
        a9n::kernel::fault_type type,
        a9n::sword              arch_fault_code,
        a9n::virtual_address    fault_address
    )
    {
        process_manager_core.retrieve_current_process()
            .transform_error(convert_kernel_to_capability_error)
            .and_then(
                [type, arch_fault_code, fault_address](process *current_process) -> capability_result
                {
                    current_process->fault_reason  = type;
                    current_process->fault_address = fault_address;
                    current_process->fault_code    = arch_fault_code;

                    if ((current_process->resolver_port.type != capability_type::IPC_PORT)
                        || !current_process->resolver_port.component) [[unlikely]]
                    {
                        // [double fault]
                        // NOTE: at the time of fault handler, current process is still the process
                        // where the fault occurred; therefore, it is necessary to execute
                        // re-schedule + switch even if resolver does not exist
                        current_process->status = process_status::BLOCKED;
                        return capability_error::INVALID_DESCRIPTOR;
                    }

                    ipc_port &port = static_cast<ipc_port &>(*current_process->resolver_port.component
                    );
                    return port
                        .operation_fault_call(*current_process, current_process->resolver_port)
                        .or_else(
                            [current_process](capability_error e) -> capability_result
                            {
                                current_process->status = process_status::BLOCKED;
                                return e;
                            }
                        );
                }
            )
            .or_else(
                [=](capability_error e) -> kernel_result
                {
                    a9n::kernel::utility::logger::printk(
                        "fault : %s, fault_code : %d, fault_address : 0x%016llx\n",
                        a9n::kernel::fault_type_to_string(type),
                        arch_fault_code,
                        fault_address
                    );

                    return process_manager_core.try_schedule_and_switch();
                }
            );
    }
}
