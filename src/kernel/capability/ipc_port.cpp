#include <kernel/capability/ipc_port.hpp>

namespace a9n::kernel
{
    capability_result ipc_port::execute(process &owner, capability_slot &self)
    {
        auto target_operation = [&]() -> operation_type
        {
            return static_cast<operation_type>(
                a9n::hal::get_message_register(owner, OPERATION_TYPE).unwrap_or(static_cast<a9n::word>(0))
            );
        };

        return get_message_info(owner)
            .transform_error(convert_kernel_to_capability_error)
            .and_then(
                [&](message_info info) -> capability_result
                {
                    switch (target_operation())
                    {
                        case SEND :
                            return operation_send(owner, self, info);

                        case RECEIVE :
                            return operation_receive(owner, self, info);

                        case CALL :
                            return operation_call(owner, self, info);

                        case REPLY :
                            return operation_receive(owner, self, info);

                        case REPLY_RECEIVE :
                            return operation_reply_receive(owner, self, info);

                        default :
                            return capability_error::ILLEGAL_OPERATION;
                    }
                }
            );
    }

    capability_result ipc_port::operation_send(process &owner, capability_slot &self, message_info info)
    {
        // send message
        switch (state)
        {
            case WAIT :
                [[fallthrough]];
            case READY_TO_SEND :
                {
                    if (!info.is_block())
                    {
                        // receiver is not ready
                        return {};
                    }
                    owner.status = process_status::BLOCKED;

                    return push_ipc_queue(owner).transform_error(
                        [](kernel_error e) -> capability_error
                        {
                            return capability_error::FATAL;
                        }
                    );
                }

            case READY_TO_RECEIVE :
                {
                    return pop_ipc_queue()
                        .transform_error(convert_kernel_to_capability_error)
                        .and_then(
                            [&](liba9n::not_null<process> target) -> capability_result
                            {
                                return transfer_message(target.get(), owner, info);
                            }
                        );
                }

            default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_result
        ipc_port::operation_receive(process &owner, capability_slot &self, message_info info)

    {
        switch (state)
        {
            case WAIT :
                [[fallthrough]];
            case READY_TO_RECEIVE :
                {
                    if (!info.is_block())
                    {
                        // receiver is not ready
                        return {};
                    }
                    owner.status = process_status::BLOCKED;

                    return push_ipc_queue(owner).transform_error(
                        [](kernel_error e) -> capability_error
                        {
                            return capability_error::FATAL;
                        }
                    );
                }

            case READY_TO_SEND :
                {
                    return pop_ipc_queue()
                        .transform_error(convert_kernel_to_capability_error)
                        .and_then(
                            [&](liba9n::not_null<process> target) -> capability_result
                            {
                                target->reply_state  = process::reply_state_object::WAIT;
                                target->reply_target = &owner;

                                return transfer_message(target.get(), owner, info);
                            }
                        );
                }

            default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_result ipc_port::operation_call(process &owner, capability_slot &self, message_info info)
    {
        // send message
        switch (state)
        {
            case WAIT :
                state = READY_TO_RECEIVE;
                [[fallthrough]];
            case READY_TO_SEND :
                {
                    if (!info.is_block())
                    {
                        // receiver is not ready
                        return {};
                    }
                    owner.status      = process_status::BLOCKED;

                    owner.reply_state = process::reply_state_object::WAIT;
                    return push_ipc_queue(owner).transform_error(
                        [](kernel_error e) -> capability_error
                        {
                            return capability_error::FATAL;
                        }
                    );
                }

            case READY_TO_RECEIVE :
                {
                    return pop_ipc_queue()
                        .transform_error(convert_kernel_to_capability_error)
                        .and_then(
                            [&](liba9n::not_null<process> target) -> capability_result
                            {
                                switch (target->reply_state)
                                {
                                    case process::reply_state_object::NONE :
                                        target->reply_state
                                            = process::reply_state_object::READY_TO_REPLY;
                                        [[fallthrough]];
                                    case process::reply_state_object::READY_TO_REPLY :
                                        // overwrite
                                        target->reply_target = &owner;
                                        return {};

                                    case process::reply_state_object::WAIT :
                                        return transfer_message(target.get(), owner, info)
                                            .and_then(
                                                [&, this](void) -> capability_result
                                                {
                                                    target->reply_state
                                                        = process::reply_state_object::READY_TO_REPLY;
                                                    target->reply_target = &owner;
                                                    return {};
                                                }
                                            );

                                    default :
                                        return capability_error::ILLEGAL_OPERATION;
                                }
                            }
                        );
                }

            default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    // non-blocking !
    capability_result
        ipc_port::operation_reply(process &owner, capability_slot &self, message_info info)
    {
        switch (owner.reply_state)
        {
            case process::reply_state_object::READY_TO_REPLY :
                {
                    // available for immediate `reply`
                    [[unlikely]] if (!owner.reply_target)
                    {
                        return capability_error::FATAL;
                    }

                    return transfer_message(*owner.reply_target, owner, info)
                        .and_then(
                            [&, this](void) -> capability_result
                            {
                                // initialize
                                owner.reply_target->reply_state = process::reply_state_object::NONE;
                                owner.reply_target->reply_target = nullptr;

                                owner.reply_state  = process::reply_state_object::NONE;
                                owner.reply_target = nullptr;

                                return {};
                            }
                        );
                }

            case process::reply_state_object::NONE :
            case process::reply_state_object::WAIT :
                return {};

            default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_result
        ipc_port::operation_reply_receive(process &owner, capability_slot &self, message_info &info)
    {
        return operation_reply(owner, self, info)
            .and_then(
                [&, this](void) -> capability_result
                {
                    return operation_receive(owner, self, info);
                }
            );
    }

    liba9n::result<ipc_port::message_info, kernel_error> ipc_port::get_message_info(process &owner)
    {
        return a9n::hal::get_message_register(owner, MESSAGE_INFO)
            .transform_error(convert_hal_to_kernel_error)
            .transform(
                [](a9n::word v) -> message_info
                {
                    return message_info(v);
                }
            );
    }

    capability_result ipc_port::transfer_message(process &receiver, process &sender, message_info info)
    {
        using enum ipc_port_state;

        // retrieves one from the queue and copies the message
        if (!is_synchronized())
        {
            return capability_error::FATAL;
        }

        return a9n::hal::get_message_register(sender, info.message_length())
            .transform_error(convert_hal_to_kernel_error)
            .and_then(
                [&](a9n::word message_length) -> kernel_result
                {
                    // if it is the end o the queue, reset the state to WAIT
                    if (!queue_head)
                    {
                        state = WAIT;
                    }

                    return copy_messages(receiver, sender, message_length);
                }
            )
            .or_else(
                [&](kernel_error e) -> capability_result
                {
                    state = WAIT;

                    return capability_error::FATAL;
                }
            );
    }

    bool ipc_port::is_synchronized(void)
    {
        if (state != WAIT)
        {
            if (!queue_head || !queue_end)
            {
                // synchronization failed;
                state = WAIT;
                return false;
            }
        }

        return true;
    }

    kernel_result ipc_port::copy_messages(
        process  &destination_process,
        process  &source_process,
        a9n::word message_length
    )
    {
        auto configure_value_from_register = [&](a9n::word index) -> a9n::hal::hal_result
        {
            index += (MESSAGE_INFO + 1); // skip descriptor, operation_type,
                                         // message_info
            return a9n::hal::get_message_register(source_process, index)
                .and_then(
                    [&](a9n::word v) -> a9n::hal::hal_result
                    {
                        return a9n::hal::configure_message_register(destination_process, index, v);
                    }
                );
        };

        for (a9n::word i = 0; i < message_length; i++)
        {
            auto result = configure_value_from_register(i);
            if (!result)
            {
                return result.transform_error(convert_hal_to_kernel_error);
            }
        }

        return {};
    }

    kernel_result ipc_port::push_ipc_queue(process &target_process)
    {
        // add to queue end
        if (!queue_head || !queue_end)
        {
            queue_head                       = &target_process;
            queue_end                        = &target_process;
            target_process.preview_ipc_queue = nullptr;
            target_process.next_ipc_queue    = nullptr;
        }
        else
        {
            queue_end->next_ipc_queue        = &target_process;
            target_process.preview_ipc_queue = queue_end;
            queue_end                        = &target_process;
        }

        return {};
    }

    liba9n::result<liba9n::not_null<process>, kernel_error> ipc_port::pop_ipc_queue(void)
    {
        // target (queue_head) null check is already done
        auto target               = queue_head;
        queue_head                = queue_head->next_ipc_queue;
        target->next_ipc_queue    = nullptr;
        target->preview_ipc_queue = nullptr;

        if (!target)
        {
            return kernel_error::NO_SUCH_ADDRESS;
        }

        return liba9n::not_null<process> { *target };
    }
}
