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
    capability_result notification_port::operation_notify(process &owner, capability_slot &self)
    {
        if (!(self.rights & capability_slot::WRITE)) [[unlikely]]
        {
            return capability_error::PERMISSION_DENIED;
        }

        // identifier is slot-local
        auto identifier = convert_slot_data_to_identifier(self.data);
        core.notify(identifier);
        DEBUG_LOG("notification_port::notify : 0x%llx", identifier);

        if (binded_process && binded_process->status == process_status::BLOCKED_RECEIVE)
        {
            return try_wake_binded_process(owner);
        }

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
                                return a9n::hal::configure_message_register(
                                           target.get(),
                                           FLAG_WORD,
                                           core.consume()
                                )
                                    .transform_error(convert_hal_to_capability_error)
                                    .and_then(
                                        [&](void) -> capability_result
                                        {
                                            return mark_scheduled_with_preemption(owner, target.get())
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
        if (!(self.rights & capability_slot::READ)) [[unlikely]]
        {
            return capability_error::PERMISSION_DENIED;
        }

        if (binded_process && binded_process != &owner) [[unlikely]]
        {
            return capability_error::FATAL;
        }

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
                    owner.status = process_status::BLOCKED_WAIT;

                    DEBUG_LOG("notification_port::wait : push to waitqueue");
                    return push_notification_queue(owner)
                        .and_then(
                            [&](void) -> kernel_result
                            {
                                DEBUG_LOG("notification_port::wait : schedule another process");
                                return try_schedule_and_switch(owner);
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
        if (!(self.rights & capability_slot::READ)) [[unlikely]]
        {
            return capability_error::PERMISSION_DENIED;
        }

        if (binded_process && binded_process != &owner) [[unlikely]]
        {
            return capability_error::FATAL;
        }

        switch (state)
        {
            case notification_port_state::WAIT :
            case notification_port_state::READY_TO_WAKE :
                {
                    if (!core.has_notification())
                    {
                        DEBUG_LOG("notification_port::poll : no notifications");
                        // write "0" to flag word if no notification is available
                        return a9n::hal::configure_message_register(owner, FLAG_WORD, 0)
                            .transform_error(convert_hal_to_capability_error);
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

    capability_result notification_port::try_wake_binded_process(process &current)
    {
        if (!binded_process) [[unlikely]]
        {
            return capability_error::FATAL;
        }

        auto &target_process = *binded_process;

        if (target_process.status != process_status::BLOCKED_RECEIVE) [[unlikely]]
        {
            return capability_error::FATAL;
        }

        if (!target_process.current_ipc_port) [[unlikely]]
        {
            return capability_error::FATAL;
        }

        auto *target_ipc_port = target_process.current_ipc_port;

        return target_ipc_port->remove_ipc_queue(target_process)
            .transform_error(convert_kernel_to_capability_error)
            .and_then(
                [&]() -> capability_result
                {
                    ipc_port::message_info
                        info { false, 0, 0, ipc_port::message_info::message_source::NOTIFICATION };

                    auto notification_identifier = core.consume();

                    return a9n::hal::configure_message_register(
                               target_process,
                               ipc_port::operation_index::MESSAGE_INFO,
                               info.data
                    )
                        .transform_error(convert_hal_to_capability_error)
                        .and_then(
                            [&]() -> capability_result
                            {
                                return a9n::hal::configure_message_register(
                                           target_process,
                                           ipc_port::operation_index::IDENTIFIER_DESTINATION,
                                           notification_identifier
                                )
                                    .transform_error(convert_hal_to_capability_error);
                            }
                        )
                        .and_then(
                            [&]() -> capability_result
                            {
                                target_process.status                    = process_status::READY;
                                target_process.current_ipc_port          = nullptr;
                                target_process.current_notification_port = nullptr;

                                return mark_scheduled(current, target_process)
                                    .transform_error(convert_kernel_to_capability_error);
                            }
                        );
                }
            );
    }

    kernel_result notification_port::bind_process(process &target_process)
    {
        if (binded_process && binded_process != &target_process) [[unlikely]]
        {
            return kernel_error::ILLEGAL_ARGUMENT;
        }

        binded_process = &target_process;

        return {};
    }

    kernel_result notification_port::unbind_process(process &target_process)
    {
        if (binded_process != &target_process) [[unlikely]]
        {
            return kernel_error::ILLEGAL_ARGUMENT;
        }

        binded_process = nullptr;

        return {};
    }

    kernel_result notification_port::push_notification_queue(process &target_process)
    {
        target_process.current_notification_port = this;

        // add to queue end
        if (!queue_head || !queue_tail)
        {
            queue_head                                = &target_process;
            queue_tail                                = &target_process;
            target_process.preview_notification_queue = nullptr;
            target_process.next_notification_queue    = nullptr;
        }
        else
        {
            target_process.next_notification_queue    = nullptr;
            queue_tail->next_notification_queue       = &target_process;
            target_process.preview_notification_queue = queue_tail;
            queue_tail                                = &target_process;
        }

        return {};
    }

    liba9n::result<liba9n::not_null<process>, kernel_error>
        notification_port::pop_notification_queue(void)
    {
        auto target = queue_head;
        if (!target) [[unlikely]]
        {
            return kernel_error::NO_SUCH_ADDRESS;
        }

        queue_head = queue_head->next_notification_queue;

        if (!queue_head)
        {
            queue_tail = nullptr;
            state      = WAIT;
        }
        else
        {
            queue_head->preview_notification_queue = nullptr;
        }

        target->next_notification_queue    = nullptr;
        target->preview_notification_queue = nullptr;
        target->current_notification_port  = nullptr;

        return liba9n::not_null<process> { *target };
    }

    kernel_result notification_port::remove_notification_queue(process &target_process)
    {
        if (target_process.current_notification_port != this) [[unlikely]]
        {
            return kernel_error::ILLEGAL_ARGUMENT;
        }

        if (target_process.preview_notification_queue)
        {
            target_process.preview_notification_queue->next_notification_queue
                = target_process.next_notification_queue;
        }
        else
        {
            queue_head = target_process.next_notification_queue;
        }

        if (target_process.next_notification_queue)
        {
            target_process.next_notification_queue->preview_notification_queue
                = target_process.preview_notification_queue;
        }
        else
        {
            queue_tail = target_process.preview_notification_queue;
        }

        target_process.next_notification_queue    = nullptr;
        target_process.preview_notification_queue = nullptr;
        target_process.current_notification_port  = nullptr;

        if (!queue_head)
        {
            queue_tail = nullptr;
            state      = WAIT;
        }

        return {};
    }

}
