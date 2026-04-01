#include <kernel/capability/notification_port.hpp>

#include <hal/interface/process_manager.hpp>
#include <kernel/process/process_manager.hpp>
#include <kernel/utility/logger.hpp>

namespace a9n::kernel
{
    // utility
    namespace
    {
        inline constexpr capability_error
            convert_hal_to_capability_error([[maybe_unused]] a9n::hal::hal_error e)
        {
            return capability_error::FATAL;
        }
    }

    capability_result notification_port::execute(process &this_process, capability_slot &this_slot)
    {
        auto target_operation = [&](void) -> operation_type
        {
            return static_cast<operation_type>(
                a9n::hal::get_message_register(this_process, OPERATION_TYPE)
                    .unwrap_or(static_cast<a9n::word>(0))
            );
        };

        switch (target_operation())
        {
            case operation_type::OPERATION_NOTIFY :
                DEBUG_LOG("notification_port::notify");
                return operation_notify(this_process, this_slot);

            case operation_type::OPERATION_WAIT :
                DEBUG_LOG("notification_port::wait");
                return operation_wait(this_process, this_slot);

            case operation_type::OPERATION_POLL :
                DEBUG_LOG("notification_port::poll");
                return operation_poll(this_process, this_slot);

            case operation_type::OPERATION_IDENTIFY :
                DEBUG_LOG("notification_port::identify");
                return operation_identify(this_process, this_slot);

            [[unlikely]] default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    // non-blocking !
    capability_result
        notification_port::operation_notify([[maybe_unused]] process &owner, capability_slot &self)
    {
        // identifier is slot-local
        auto identifier = convert_slot_data_to_identifier(self.data);
        core.notify(identifier);
        DEBUG_LOG("notification_port::notify : 0x%llx", identifier);

        switch (state)
        {
            case notification_port_state::WAIT :
                {
                    DEBUG_LOG("notification_port::notify : no waiters");
                    // notify is always non-blocking; and it is always successful
                    break;
                }

            case notification_port_state::READY_TO_WAKE :
                {
                    DEBUG_LOG("notification_port::notify : wake up a waiter");
                    if (!queue_head) [[unlikely]]
                    {
                        // not properly initialized
                        return capability_error::FATAL;
                    }

                    return pop_notification_queue()
                        .transform_error(convert_kernel_to_capability_error)
                        .and_then(
                            [&](liba9n::not_null<process> target) -> capability_result
                            {
                                target->status = process_status::READY;

                                return a9n::hal::configure_message_register(
                                           target.get(),
                                           FLAG_WORD,
                                           core.consume()
                                )
                                    .transform_error(convert_hal_to_capability_error)
                                    .and_then(
                                        [&](void) -> capability_result
                                        {
                                            return process_manager_core
                                                .try_direct_schedule_and_switch(target.get())
                                                .transform_error(convert_kernel_to_capability_error);
                                        }
                                    );
                            }
                        );
                }
        }

        return {};
    }

    capability_result notification_port::operation_wait(process &owner, capability_slot &self)
    {
        DEBUG_LOG("notification_port::wait");
        switch (state)
        {
            case notification_port_state::WAIT :
                {
                    DEBUG_LOG("notification_port::wait : no notifications, block process");
                    if (core.has_notification())
                    {
                        DEBUG_LOG("notification_port::wait : consume notification without blocking");
                        return a9n::hal::configure_message_register(owner, FLAG_WORD, core.consume())
                            .transform_error(convert_hal_to_capability_error);
                    }
                    [[fallthrough]];
                }
            case notification_port_state::READY_TO_WAKE :
                {
                    DEBUG_LOG("notification_port::wait : block process");
                    state        = notification_port_state::READY_TO_WAKE;
                    owner.status = process_status::BLOCKED;

                    DEBUG_LOG("notification_port::wait : push to waitqueue");
                    return push_notification_queue(owner)
                        .and_then(
                            [&](void) -> kernel_result
                            {
                                DEBUG_LOG("notification_port::wait : schedule another process");
                                return process_manager_core.try_schedule_and_switch();
                            }
                        )
                        .transform_error(convert_kernel_to_capability_error);
                }

            [[unlikely]] default :
                DEBUG_LOG("notification_port::wait : invalid state");
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    // non-blocking !
    capability_result notification_port::operation_poll(process &owner, capability_slot &self)
    {
        switch (state)
        {
            case notification_port_state::WAIT :
            case notification_port_state::READY_TO_WAKE :
                {
                    if (!core.has_notification())
                    {
                        DEBUG_LOG("notification_port::poll : no notifications");
                        return {};
                    }

                    return a9n::hal::configure_message_register(owner, FLAG_WORD, core.consume())
                        .transform_error(convert_hal_to_capability_error);
                }

            [[unlikely]] default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_result notification_port::operation_identify(process &owner, capability_slot &self)
    {
        if (!(self.rights & capability_slot::object_rights::MODIFY))
        {
            return capability_error::PERMISSION_DENIED;
        }

        return a9n::hal::get_message_register(owner, NEW_IDENTIFIER)
            .transform_error(convert_hal_to_capability_error)
            .and_then(
                [&](a9n::word identifier) -> capability_result
                {
                    self.data = convert_identifier_to_slot_data(identifier);
                    return {};
                }
            );
    }

    kernel_result notification_port::push_notification_queue(process &target_process)
    {
        // add to queue end
        if (!queue_head || !queue_tail)
        {
            queue_head                       = &target_process;
            queue_tail                       = &target_process;
            target_process.preview_ipc_queue = nullptr;
            target_process.next_ipc_queue    = nullptr;
        }
        else
        {
            queue_tail->next_ipc_queue       = &target_process;
            target_process.preview_ipc_queue = queue_tail;
            queue_tail                       = &target_process;
        }

        return {};
    }

    liba9n::result<liba9n::not_null<process>, kernel_error>
        notification_port::pop_notification_queue(void)
    {
        // target (queue_head) null check is already done
        auto target = queue_head;
        queue_head  = queue_head->next_ipc_queue;

        if (!queue_head)
        {
            queue_tail = nullptr;
            state      = WAIT;
        }

        if (!target) [[unlikely]]
        {
            return kernel_error::NO_SUCH_ADDRESS;
        }

        target->next_ipc_queue    = nullptr;
        target->preview_ipc_queue = nullptr;

        return liba9n::not_null<process> { *target };
    }

}
