#include <kernel/kernelcall/kernel_call.hpp>

#include <kernel/types.hpp>

#include <kernel/process/process_manager.hpp>
#include <kernel/utility/logger.hpp>

#include <kernel/capability/capability_result.hpp>

namespace a9n::kernel
{
    // called from hal's system call handler
    void handle_kernel_call(
        /*
        a9n::capability_descriptor descriptor_with_depth,
        a9n::word                  message_tag,
        a9n::word                  message_length
        */
        kernel_call_type type
    )
    {
        auto result = process_manager_core.retrieve_current_process().and_then(
            [=](process *current_process) -> kernel_result
            {
                switch (type)
                {
                    using enum kernel_call_type;
                    case CAPABILITY_CALL :
                        return handle_capability_call(*current_process);

                    case YIELD :
                        return handle_yield(*current_process);

                    case DEBUG :
                        return handle_debug_call(*current_process);

                    default :
                        return kernel_error::ILLEGAL_ARGUMENT;
                }

                return kernel_error::UNEXPECTED;
            }
        );

        if (!result)
        {
            for (;;)
                ;
        }

        return;

        // TODO: implement judgement-system
    }

    kernel_result handle_capability_call(process &current_process)
    {
        [[unlikely]] if (!current_process.root_slot.component)
        {
            a9n::kernel::utility::logger::printk("capability slot is empty\n");
            return {};
        }

        // TODO: parse descriptor_with_depth -> descriptor / depth

        auto result = current_process.root_slot.component->traverse_slot(0, 0, 0);
        if (!result)
        {
            a9n::kernel::utility::logger::printk(
                "lookup error : %s\n",
                capability_lookup_error_to_string(result.unwrap_error())
            );

            return kernel_error::TRY_AGAIN;
        }

        auto cap_result = result.unwrap()->component->execute(current_process, *result.unwrap());

        if (!result)
        {
            a9n::kernel::utility::logger::printk(
                "capability error : %s\n",
                capability_error_to_string(cap_result.unwrap_error())
            );

            return kernel_error::TRY_AGAIN;
        }

        return {};
    }

    kernel_result handle_yield(process &current_process)
    {
        current_process.quantum = 0;
        return process_manager_core.switch_context();
    }

    kernel_result handle_debug_call(process &current_process)
    {
        // TODO: do error-handling
        auto raw_charactor = a9n::hal::get_message_register(current_process, 0);
        auto charactor     = static_cast<uint8_t>(raw_charactor.unwrap());
        kernel::utility::logger::put_char(charactor);

        return {};
    }
}
