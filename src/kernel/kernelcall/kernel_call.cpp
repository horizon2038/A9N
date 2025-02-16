#include <kernel/kernelcall/kernel_call.hpp>

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_result.hpp>
#include <kernel/interrupt/interrupt_manager.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/process/process_manager.hpp>
#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

#include <hal/interface/process_manager.hpp>

#include <liba9n/common/enum.hpp>

namespace a9n::kernel
{
    // called from hal's system call handler
    void handle_kernel_call(kernel_call_type type)
    {
        auto result = process_manager_core.retrieve_current_process().and_then(
            [type](process *current_process) -> kernel_result
            {
                switch (type)
                {
                    using enum kernel_call_type;
                    [[likely]] case CAPABILITY_CALL :
                        // DEBUG_LOG("capability call");
                        return handle_capability_call(*current_process);

                    case YIELD :
                        DEBUG_LOG("yield");
                        return handle_yield(*current_process);

                    case DEBUG :
                        // DEBUG_LOG("debug call");
                        return handle_debug_call(*current_process);

                    default :
                        // usually unreachable
                        return kernel_error::UNEXPECTED;
                }
            }
        );

        if (!result) [[unlikely]]
        {
            DEBUG_LOG("failed kernel call");
            /*
            for (;;)
                ;
            */
        }
    }

    inline void configure_capability_error(process &current_process, capability_error error)
    {
        a9n::hal::configure_message_register(current_process, 0, 0);
        a9n::hal::configure_message_register(current_process, 1, liba9n::enum_cast(error));
    }

    inline void configure_capability_success(process &current_process)
    {
        a9n::hal::configure_message_register(current_process, 0, 1);
    }

    inline kernel_result handle_capability_error(process &current_process, capability_error error)
    {
        configure_capability_error(current_process, error);
        [[unlikely]] if (error == capability_error::FATAL)
        {
            return kernel_error::UNEXPECTED;
        }

        return kernel_error::TRY_AGAIN;
    }

    // TODO: clean this
    kernel_result handle_capability_call(process &current_process)
    {
        if (!current_process.root_slot.component) [[unlikely]]
        {
            a9n::kernel::utility::logger::error("capability slot is empty\n");
            return kernel_error::INIT_FIRST;
        }

        // +--------------+-------------------+
        // |     8bit     |  remaining bits   |
        // +--------------+-------------------+
        // | target depth | target descriptor |
        // +--------------+-------------------+

        // clang-format off
        const a9n::word raw_descriptor = a9n::hal::get_message_register(current_process, 0).unwrap_or(static_cast<a9n::word>(0));
        const a9n::word depth          = extract_depth(raw_descriptor);
        //clang-format on

        DEBUG_LOG("raw descriptor : 0x%016llx", raw_descriptor);
        DEBUG_LOG("depth : %8llu", depth);

        auto result
            = current_process.root_slot.component->traverse_slot(raw_descriptor, depth, a9n::BYTE_BITS);
        if (!result) [[unlikely]]
        {
            a9n::kernel::utility::logger::printk(
                "lookup error : %s\n",
                capability_lookup_error_to_string(result.unwrap_error())
            );
            configure_capability_error(current_process, capability_error::INVALID_DESCRIPTOR);

            return kernel_error::TRY_AGAIN;
        }

        capability_slot &slot = *result.unwrap();

        if (slot.type == capability_type::NONE) [[unlikely]]
        {
            return handle_capability_error(current_process, capability_error::INVALID_DESCRIPTOR);
        }

        DEBUG_LOG("capability_slot : 0x%16llx", &slot);
        DEBUG_LOG("capability_type : 0x%16llx", slot.type);

        return slot.component->execute(current_process, slot)
            .and_then(
                [&](void) -> capability_result
                {
                    configure_capability_success(current_process);
                    return {};
                }
            )
            .or_else(
                [&](capability_error e) -> kernel_result
                {
                    DEBUG_LOG(
                        "capability error : %s",
                        capability_error_to_string(e)
                    );

                    configure_capability_error(current_process, e);

                    if (e == capability_error::FATAL) [[unlikely]] 
                    {
                        return kernel_error::UNEXPECTED;
                    }

                    DEBUG_LOG("return to ...");
                    return {};
                }
            );
    }

    kernel_result handle_yield(process &current_process)
    {
        return process_manager_core.yield();
    }

    kernel_result handle_debug_call(process &current_process)
    {
        auto raw_charactor = a9n::hal::get_message_register(current_process, 0);
        auto charactor     = static_cast<uint8_t>(raw_charactor.unwrap());
        kernel::utility::logger::put_char(charactor);

        return {};
    }
}
