#include "hal/interface/process_manager.hpp"
#include "kernel/types.hpp"
#include <kernel/capability/ipc_port.hpp>

#include <kernel/capability/capability_result.hpp>
#include <kernel/capability/capability_utility.hpp>
#include <kernel/interrupt/fault.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/process/process_manager.hpp>

#include <kernel/capability/capability_invocation.hpp>
#include <kernel/capability/capability_utility.hpp>
#include <kernel/capability/notification_port.hpp>
#include <kernel/utility/logger.hpp>

namespace a9n::kernel
{
    // helper
    namespace
    {
        inline kernel_result try_schedule_and_switch(void)
        {
            return process_manager_core.try_schedule_and_switch();
        }

    }

    capability_result ipc_port::execute(process &owner, capability_slot &self)
    {
        DEBUG_LOG("ipc_port::execute");
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
                    if (!info.is_normal()) [[unlikely]]
                    {
                        DEBUG_LOG("invalid message source for ipc_port::execute");
                        return capability_error::INVALID_ARGUMENT;
                    }

                    switch (target_operation())
                    {
                        case SEND :
                            DEBUG_LOG("ipc_port::send");
                            return operation_send(owner, self, info);

                        case RECEIVE :
                            DEBUG_LOG("ipc_port::receive");
                            return operation_receive(owner, self, info);

                        [[likely]] case CALL :
                            DEBUG_LOG("ipc_port::call");
                            return operation_call(owner, self, info);

                        case REPLY :
                            DEBUG_LOG("ipc_port::reply");
                            return operation_reply(owner, self, info);

                        [[likely]] case REPLY_RECEIVE :
                            DEBUG_LOG("ipc_port::reply_receive");
                            return operation_reply_receive(owner, self, info);

                        case IDENTIFY :
                            DEBUG_LOG("ipc_port::identify");
                            return operation_identify(owner, self);

                        [[unlikely]] default :
                            DEBUG_LOG("illegal operation!");
                            return capability_error::ILLEGAL_OPERATION;
                    }
                }
            );
    }

    capability_result ipc_port::operation_send(process &owner, capability_slot &self, message_info info)
    {
        if (!(self.rights & capability_slot::WRITE)) [[unlikely]]
        {
            return capability_error::PERMISSION_DENIED;
        }

        // send message
        switch (state)
        {
            case WAIT :
                state = READY_TO_SEND;
                [[fallthrough]];
            case READY_TO_SEND :
                // The sender is ready, but there is no recipient. Waiting until a recipient appears.
                {
                    if (!info.is_block()) [[unlikely]]
                    {
                        // receiver is not ready
                        return {};
                    }

                    owner.status                  = process_status::BLOCKED_SEND;
                    owner.identifier_when_blocked = convert_slot_data_to_identifier(self.data);

                    return push_ipc_queue(owner)
                        .and_then(try_schedule_and_switch)
                        .transform_error(
                            [](kernel_error e) -> capability_error
                            {
                                DEBUG_LOG("kernel_error : %s", kernel_error_to_string(e));
                                return capability_error::FATAL;
                            }
                        );
                }

            case READY_TO_RECEIVE :
                // The receiver exists, so the message will be sent as is.
                {
                    return pop_ipc_queue()
                        .transform_error(convert_kernel_to_capability_error)
                        .and_then(
                            [&](liba9n::not_null<process> target) -> capability_result
                            {
                                target->status = process_status::READY;
                                owner.identifier_when_blocked
                                    = convert_slot_data_to_identifier(self.data);

                                return transfer_message(target.get(), owner, info)
                                    .and_then(
                                        [&](void) -> capability_result
                                        {
                                            // re-queueing
                                            return process_manager_core.mark_scheduled(target.get())
                                                .transform_error(convert_kernel_to_capability_error);
                                        }
                                    );
                            }
                        );
                }

            [[unlikely]] default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_result
        ipc_port::operation_receive(process &owner, capability_slot &self, message_info info)

    {
        if (!(self.rights & capability_slot::READ)) [[unlikely]]
        {
            return capability_error::PERMISSION_DENIED;
        }

        // NOTE:
        // this discard path is intentionally only for RECEIVE, not REPLY_RECEIVE.
        //
        // in a normal reply-receive event loop, user code runs after the receive part has
        // completed. Therefore, when the server enters REPLY_RECEIVE again, the old reply
        // target is replied to before the next receive is performed. There is no point at
        // which REPLY_RECEIVE should discard the existing reply target.
        //
        // if the server wants to drop the current caller without replying, it must enter
        // RECEIVE again. In that case, RECEIVE discards the old reply target before waiting
        // for or accepting the next sender.
        if (owner.destination_reply_state != process::destination_reply_state_object::NONE)
            [[unlikely]]
        {
            if (owner.destination_reply_state
                != process::destination_reply_state_object::READY_TO_REPLY) [[unlikely]]
            {
                return capability_error::FATAL;
            }

            auto *client = owner.destination_reply_target;
            if (client)
            {
                client->source_reply_state  = process::source_reply_state_object::NONE;
                client->source_reply_target = nullptr;
            }

            owner.destination_reply_state  = process::destination_reply_state_object::NONE;
            owner.destination_reply_target = nullptr;
        }

        switch (state)
        {
            case WAIT :
                DEBUG_LOG("WAIT");
                state = READY_TO_RECEIVE;
                [[fallthrough]];
            case READY_TO_RECEIVE :
                // The receiver (self) is ready, but there is no sender. Waiting until a sender
                // appears.
                {
                    DEBUG_LOG("READY_TO_RECEIVE");
                    if (!info.is_block()) [[unlikely]]
                    {
                        // receiver is not ready
                        return {};
                    }

                    owner.status = process_status::BLOCKED_RECEIVE;

                    return push_ipc_queue(owner)
                        .and_then(try_schedule_and_switch)
                        .transform_error(convert_kernel_to_capability_error);
                }

            case READY_TO_SEND :
                // The sender exists, so the message will be received as is.
                {
                    DEBUG_LOG("READY_TO_SEND");
                    return try_receive_from_ready_sender(owner);
                }

            [[unlikely]] default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_result ipc_port::operation_call(process &owner, capability_slot &self, message_info info)
    {
        if (!(self.rights & capability_slot::WRITE)) [[unlikely]]
        {
            return capability_error::PERMISSION_DENIED;
        }

        // send message
        switch (state)
        {
            case WAIT :
                state = READY_TO_SEND;
                [[fallthrough]];
            case READY_TO_SEND :
                // The sender is ready, but there is no recipient. Waiting until a recipient appears.
                {
                    if (!info.is_block()) [[unlikely]]
                    {
                        // receiver is not ready
                        return {};
                    }

                    // TODO: optimize this path
                    bool delivered_notification = false;
                    if (auto notification_result
                        = try_deliver_pending_binded_notification(owner, delivered_notification);
                        !notification_result) [[unlikely]]
                    {
                        return notification_result;
                    }
                    if (delivered_notification) [[unlikely]]
                    {
                        return {};
                    }

                    owner.status                  = process_status::BLOCKED_SEND;
                    owner.source_reply_state      = process::source_reply_state_object::WAIT;
                    owner.identifier_when_blocked = convert_slot_data_to_identifier(self.data);

                    return push_ipc_queue(owner)
                        .and_then(try_schedule_and_switch)
                        .transform_error(convert_kernel_to_capability_error);
                }

            [[likely]] case READY_TO_RECEIVE :
                // The receiver exists, so the message will be sent as is. However, the sender will
                // be blocked until the reply is handled, since it's a call operation.
                {
                    return pop_ipc_queue()
                        .transform_error(convert_kernel_to_capability_error)
                        .and_then(
                            [&](liba9n::not_null<process> target) -> capability_result
                            {
                                owner.identifier_when_blocked
                                    = convert_slot_data_to_identifier(self.data);

                                if (target->destination_reply_state
                                    != process::destination_reply_state_object::NONE) [[unlikely]]
                                {
                                    return capability_error::FATAL;
                                }

                                target->destination_reply_state
                                    = process::destination_reply_state_object::READY_TO_REPLY;
                                target->destination_reply_target = &owner;

                                owner.status                     = process_status::BLOCKED_REPLY;
                                owner.source_reply_state = process::source_reply_state_object::WAIT;
                                owner.source_reply_target = &target.get();

                                return transfer_message(target.get(), owner, info)
                                    .and_then(
                                        [&, this](void) -> capability_result
                                        {
                                            target->status = process_status::READY;
                                            return process_manager_core
                                                .try_direct_schedule_and_switch(target.get())
                                                .transform_error(convert_kernel_to_capability_error);

                                            return process_manager_core.try_schedule_and_switch()
                                                .transform_error(convert_kernel_to_capability_error);
                                        }
                                    );
                            }
                        );
                }

            [[unlikely]] default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    // non-blocking !
    capability_result
        ipc_port::operation_reply(process &owner, capability_slot &self, message_info info)
    {
        // reply does not require any rights !
        switch (owner.destination_reply_state)
        {
            [[likely]] case process::destination_reply_state_object::READY_TO_REPLY :
                {
                    DEBUG_LOG("ready to reply");
                    // available for immediate `reply`
                    if (!owner.destination_reply_target) [[unlikely]]
                    {
                        return capability_error::FATAL;
                    }

                    DEBUG_LOG("destination_reply_target : 0x%016llx", owner.destination_reply_target);
                    auto client = owner.destination_reply_target;

                    return transfer_direct_message(*client, owner, info)
                        .and_then(
                            [&](void) -> capability_result
                            {
                                DEBUG_LOG("initialize reply_target");

                                // init client
                                client->status = process_status::READY;
                                client->source_reply_state = process::source_reply_state_object::NONE;
                                client->source_reply_target = nullptr;

                                // NOTE:
                                // This code is on a critical path, so it requires investigation for
                                // performance optimization.
                                if (client->fault_reason != fault_type::NONE) [[unlikely]]
                                {
                                    DEBUG_LOG(
                                        "client has fault reason : %s",
                                        fault_type_to_string(client->fault_reason)
                                    );
                                    // reset fault reason to NONE, since the fault is already
                                    // handled by the reply
                                    client->fault_reason = fault_type::NONE;

                                    // TODO: Implement replies for each fault reason if needed
                                }

                                // init server (this)
                                owner.destination_reply_state
                                    = process::destination_reply_state_object::NONE;
                                owner.destination_reply_target = nullptr;

                                return process_manager_core.mark_scheduled(*client)
                                    .transform_error(convert_kernel_to_capability_error)
                                    .and_then(
                                        [&](void) -> capability_result
                                        {
                                            owner.status = process_status::READY;

                                            return process_manager_core
                                                .try_direct_schedule_and_switch(owner)
                                                .transform_error(convert_kernel_to_capability_error);
                                        }
                                    );
                            }
                        );
                }

            case process::destination_reply_state_object::NONE :
                return {};

            [[unlikely]] default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_result
        ipc_port::operation_reply_receive(process &owner, capability_slot &self, message_info info)
    {
        if (!(self.rights & capability_slot::READ)) [[unlikely]]
        {
            return capability_error::PERMISSION_DENIED;
        }

        return complete_reply_without_switch(owner, info)
            .and_then(
                [&]() -> capability_result
                {
                    switch (state)
                    {
                        [[likely]] case READY_TO_SEND :
                            {
                                return try_receive_from_ready_sender(owner);
                            }

                        [[unlikely]] case WAIT :
                            {
                                state = READY_TO_RECEIVE;
                                [[fallthrough]];
                            }

                        [[unlikely]] case READY_TO_RECEIVE :
                            {
                                bool delivered_notification = false;
                                if (auto notification_result
                                    = try_deliver_pending_binded_notification(owner, delivered_notification);
                                    !notification_result)
                                {
                                    return notification_result;
                                }
                                if (delivered_notification) [[unlikely]]
                                {
                                    return {};
                                }

                                if (!info.is_block()) [[unlikely]]
                                {
                                    return {};
                                }

                                owner.status = process_status::BLOCKED_RECEIVE;

                                return push_ipc_queue(owner)
                                    .and_then(try_schedule_and_switch)
                                    .transform_error(convert_kernel_to_capability_error);
                            }

                        [[unlikely]] default :
                            {
                                return capability_error::FATAL;
                            }
                    }
                }
            );
    }

    capability_result ipc_port::complete_reply_without_switch(process &owner, message_info info)
    {
        if (owner.destination_reply_state == process::destination_reply_state_object::NONE)
        {
            return {};
        }

        if (owner.destination_reply_state != process::destination_reply_state_object::READY_TO_REPLY)
            [[unlikely]]
        {
            return capability_error::FATAL;
        }

        auto *client = owner.destination_reply_target;
        if (!client) [[unlikely]]
        {
            return capability_error::FATAL;
        }

        return transfer_direct_message(*client, owner, info)
            .and_then(
                [&]() -> capability_result
                {
                    client->status              = process_status::READY;
                    client->source_reply_state  = process::source_reply_state_object::NONE;
                    client->source_reply_target = nullptr;

                    if (client->fault_reason != fault_type::NONE) [[unlikely]]
                    {
                        client->fault_reason = fault_type::NONE;
                    }

                    owner.destination_reply_state  = process::destination_reply_state_object::NONE;
                    owner.destination_reply_target = nullptr;

                    return process_manager_core.mark_scheduled(*client).transform_error(
                        convert_kernel_to_capability_error
                    );
                }
            );
    }

    capability_result ipc_port::try_receive_from_ready_sender(process &receiver)
    {
        if (state != READY_TO_SEND) [[unlikely]]
        {
            return capability_error::FATAL;
        }

        return pop_ipc_queue()
            .transform_error(convert_kernel_to_capability_error)
            .and_then(
                [&](liba9n::not_null<process> sender) -> capability_result
                {
                    auto sender_info_result = get_message_info(sender.get());
                    if (!sender_info_result) [[unlikely]]
                    {
                        return convert_kernel_to_capability_error(sender_info_result.unwrap_error());
                    }

                    auto sender_info = sender_info_result.unwrap();

                    if (receiver.destination_reply_state
                        != process::destination_reply_state_object::NONE) [[unlikely]]
                    {
                        return capability_error::FATAL;
                    }

                    if (sender->fault_reason != fault_type::NONE) [[unlikely]]
                    {
                        return transfer_fault_message(receiver, sender.get())
                            .and_then(
                                [&]() -> capability_result
                                {
                                    receiver.destination_reply_state
                                        = process::destination_reply_state_object::READY_TO_REPLY;
                                    receiver.destination_reply_target = &sender.get();

                                    sender->status = process_status::BLOCKED_REPLY;

                                    return {};
                                }
                            );
                    }

                    const bool is_call_sender
                        = sender->source_reply_state == process::source_reply_state_object::WAIT;

                    return transfer_message(receiver, sender.get(), sender_info)
                        .and_then(
                            [&]() -> capability_result
                            {
                                if (is_call_sender)
                                {
                                    receiver.destination_reply_state
                                        = process::destination_reply_state_object::READY_TO_REPLY;
                                    receiver.destination_reply_target = &sender.get();

                                    sender->status = process_status::BLOCKED_REPLY;

                                    return {};
                                }

                                receiver.destination_reply_state
                                    = process::destination_reply_state_object::NONE;
                                receiver.destination_reply_target = nullptr;

                                sender->status                    = process_status::READY;

                                return process_manager_core.mark_scheduled(sender.get())
                                    .transform_error(convert_kernel_to_capability_error);
                            }
                        );
                }
            );
    }

    capability_result ipc_port::try_deliver_pending_binded_notification(process &owner, bool &delivered)
    {
        delivered = false;

        if (owner.binded_notification_port.type != capability_type::NOTIFICATION_PORT
            || !owner.binded_notification_port.component) [[likely]]
        {
            return {};
        }

        auto *port = static_cast<notification_port *>(owner.binded_notification_port.component);
        if (!port->has_pending_notification()) [[likely]]
        {
            return {};
        }

        // slowpath
        message_info info {
            false,
            0,
            0,
            ipc_port::message_info::message_source::NOTIFICATION,
        };
        auto identifier = port->consume_notification();

        delivered       = true;
        return a9n::hal::configure_message_register(owner, ipc_port::operation_index::MESSAGE_INFO, info.data)
            .transform_error(convert_hal_to_kernel_error)
            .transform_error(convert_kernel_to_capability_error)
            .and_then(
                [&](void) -> capability_result
                {
                    return a9n::hal::configure_message_register(
                               owner,
                               ipc_port::operation_index::IDENTIFIER_DESTINATION,
                               identifier
                    )
                        .transform_error(convert_hal_to_kernel_error)
                        .transform_error(convert_kernel_to_capability_error);
                }
            );
    }

    capability_result ipc_port::operation_identify(process &owner, capability_slot &self)
    {
        if (!(self.rights & capability_slot::MODIFY)) [[unlikely]]
        {
            DEBUG_LOG("no MODIFY rights");
            return capability_error::PERMISSION_DENIED;
        }

        return a9n::hal::get_message_register(owner, IDENTIFIER_SOURCE)
            .transform_error(convert_hal_to_kernel_error)
            .transform_error(convert_kernel_to_capability_error)
            .and_then(
                [&self](a9n::word identifier) -> capability_result
                {
                    DEBUG_LOG("identifier : 0x%16llx to slot 0x%016llx", identifier, &self);
                    self.data = convert_identifier_to_slot_data(identifier);
                    DEBUG_LOG("identify self=%p", &self);
                    DEBUG_LOG("identify self.data[0]=0x%016llx", self.data[0]);

                    return {};
                }
            );
    }

    // NOTE: called from fault dispatcher in HAL
    // already configured fault reason in process
    // TODO: split into another file
    capability_result ipc_port::operation_fault_call(process &owner, capability_slot &self)
    {
        DEBUG_LOG(
            "ipc_port::operation_fault_call self=%p component=%p owner=%p fault_reason=%llu "
            "fault_address=%llx fault_code=%llx arch_fault_code=%llx",
            &self,
            self.component,
            &owner,
            static_cast<a9n::word>(owner.fault_reason),
            owner.fault_address,
            owner.fault_code,
            owner.arch_fault_code
        );
        DEBUG_LOG("fault self=%p", &self);
        DEBUG_LOG("fault self.data[0]=0x%016llx", self.data[0]);

        if (!(self.rights & capability_slot::WRITE) || !(self.rights & capability_slot::READ))
            [[unlikely]]
        {
            owner.status = process_status::BLOCKED_FAULT;
            return capability_error::PERMISSION_DENIED;
        }

        // configure identifier
        DEBUG_LOG("configuer identifier_when_blocked");
        owner.identifier_when_blocked = convert_slot_data_to_identifier(self.data);
        DEBUG_LOG("owner.identifier_when_blocked : 0x%016llx", owner.identifier_when_blocked);

        DEBUG_LOG("self: %p", &self);

        // send message
        switch (state)
        {
            case WAIT :
                state = READY_TO_SEND;
                [[fallthrough]];
            case READY_TO_SEND :
                {
                    owner.status                  = process_status::BLOCKED_FAULT;
                    owner.source_reply_state      = process::source_reply_state_object::WAIT;
                    owner.identifier_when_blocked = convert_slot_data_to_identifier(self.data);
                    DEBUG_LOG(
                        "owner.status set to BLOCKED, identifier_when_blocked : 0x%016llx",
                        owner.identifier_when_blocked
                    );

                    return push_ipc_queue(owner)
                        .and_then(try_schedule_and_switch)
                        .transform_error(convert_kernel_to_capability_error);
                }

            [[likely]] case READY_TO_RECEIVE :
                // The receiver exists, so the message will be sent as is. However, the sender will
                // be blocked until the reply is handled, since it's a call operation.
                {
                    return pop_ipc_queue()
                        .transform_error(convert_kernel_to_capability_error)
                        .and_then(
                            [&](liba9n::not_null<process> target) -> capability_result
                            {
                                owner.identifier_when_blocked
                                    = convert_slot_data_to_identifier(self.data);
                                DEBUG_LOG(
                                    "identifier_when_blocked : 0x%016llx",
                                    owner.identifier_when_blocked
                                );

                                [[unlikely]] if (target->destination_reply_state
                                                 != process::destination_reply_state_object::NONE)
                                {
                                    owner.status = process_status::BLOCKED_FAULT;
                                    owner.source_reply_state = process::source_reply_state_object::WAIT;
                                    return push_ipc_queue(owner)
                                        .and_then(try_schedule_and_switch)
                                        .transform_error(convert_kernel_to_capability_error);
                                }

                                target->destination_reply_state
                                    = process::destination_reply_state_object::READY_TO_REPLY;
                                target->destination_reply_target = &owner;

                                return transfer_fault_message(target.get(), owner)
                                    .and_then(
                                        [&, this](void) -> capability_result
                                        {
                                            target->status = process_status::READY;
                                            return process_manager_core
                                                .try_direct_schedule_and_switch(target.get())
                                                .transform_error(convert_kernel_to_capability_error);
                                        }
                                    );
                            }
                        );
                }

            [[unlikely]] default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    kernel_result ipc_port::remove_ipc_queue(process &target_process)
    {
        if (target_process.current_ipc_port != this) [[unlikely]]
        {
            return kernel_error::ILLEGAL_ARGUMENT;
        }

        if (target_process.preview_ipc_queue)
        {
            target_process.preview_ipc_queue->next_ipc_queue = target_process.next_ipc_queue;
        }
        else
        {
            // target_process is head
            queue_head = target_process.next_ipc_queue;
        }

        if (target_process.next_ipc_queue)
        {
            target_process.next_ipc_queue->preview_ipc_queue = target_process.preview_ipc_queue;
        }
        else
        {
            // target_process is tail
            queue_tail = target_process.preview_ipc_queue;
        }

        target_process.next_ipc_queue    = nullptr;
        target_process.preview_ipc_queue = nullptr;
        target_process.current_ipc_port  = nullptr;

        if (!queue_head)
        {
            queue_tail = nullptr;
            state      = WAIT;
        }

        return {};
    }

    liba9n::result<ipc_port::message_info, kernel_error> ipc_port::get_message_info(process &owner)
    {
        return a9n::hal::get_message_register(owner, MESSAGE_INFO)
            .transform_error(convert_hal_to_kernel_error)
            .transform(
                [](a9n::word v) -> message_info
                {
                    auto target_message_info = message_info(v);
                    DEBUG_LOG(
                        "message_info is_block : %c, message_length : %u, "
                        "transfer_count : %u, source : %u",
                        target_message_info.is_block() ? 'T' : 'F',
                        target_message_info.message_length(),
                        target_message_info.transfer_count(),
                        static_cast<a9n::word>(target_message_info.source())
                    );

                    return target_message_info;
                }
            );
    }

    capability_result ipc_port::transfer_message(process &receiver, process &sender, message_info info)
    {
        using enum ipc_port_state;

        // if it is the end of the queue, reset the state to WAIT
        if (!queue_head)
        {
            state = WAIT;
        }

        // copy info and identifier
        // NOTE: unchecked copy
        a9n::hal::configure_message_register(receiver, MESSAGE_INFO, info.data);
        a9n::hal::configure_message_register(
            receiver,
            IDENTIFIER_DESTINATION,
            sender.identifier_when_blocked
        );

        return copy_messages(receiver, sender, info.message_length())
            .or_else(
                [&](kernel_error e) -> capability_result
                {
                    state = WAIT;

                    return capability_error::FATAL;
                }
            )
            .and_then(
                [&](void) -> capability_result
                {
                    return move_capabilities(receiver, sender, info.transfer_count());
                }
            );
    }

    capability_result
        ipc_port::transfer_direct_message(process &receiver, process &sender, message_info info)
    {
        a9n::hal::configure_message_register(receiver, MESSAGE_INFO, info.data);
        a9n::hal::configure_message_register(
            receiver,
            IDENTIFIER_DESTINATION,
            sender.identifier_when_blocked
        );

        return copy_messages(receiver, sender, info.message_length())
            .or_else(
                [&](kernel_error e) -> capability_result
                {
                    return capability_error::FATAL;
                }
            )
            .and_then(
                [&](void) -> capability_result
                {
                    if (info.transfer_count() != 0) [[unlikely]]
                    {
                        return move_capabilities(receiver, sender, info.transfer_count());
                    }

                    return {};
                }
            );
    }

    // message__info for fault message is determined by the kernel, not the sender, so we don't need
    // to read it from the sender's message register.
    capability_result ipc_port::transfer_fault_message(process &receiver, process &sender)
    {
        using enum ipc_port_state;

        // retrieves one from the queue and copies the message
        if (!is_synchronized()) [[unlikely]]
        {
            return capability_error::FATAL;
        }

        // if it is the end of the queue, reset the state to WAIT
        if (!queue_head)
        {
            state = WAIT;
        }

        auto make_message_info = [](a9n::word message_length) -> message_info
        {
            return message_info {
                false,                                // block
                static_cast<uint8_t>(message_length), // message length
                0,                                    // transfer count
                message_info::message_source::FAULT   // source
            };
        };

        auto result = capability_result {};
        auto fault_pc = a9n::hal::get_general_register(sender, hal::register_type::INSTRUCTION_POINTER)
                            .unwrap_or(static_cast<a9n::word>(0));

        switch (sender.fault_reason)
        {
            case fault_type::MEMORY :
                {
                    auto info = make_message_info(fault_memory_index::MESSAGE_LENGTH);
                    DEBUG_LOG(
                        "message_info : block=%d, message_length=%d, transfer_count=%d, source=%u",
                        info.is_block(),
                        info.message_length(),
                        info.transfer_count(),
                        static_cast<a9n::word>(info.source())
                    );
                    DEBUG_LOG("message_info.data : 0x%016llx", info.data);
                    result = write_message_registers<
                        fault_index::IS_SUCCESS,
                        fault_index::ERROR_CODE,
                        fault_index::MESSAGE_INFO,
                        fault_index::IDENTIFIER,
                        fault_index::FAULT_REASON,
                        fault_memory_index::FAULT_PROGRAM_COUNTER,
                        fault_memory_index::FAULT_ADDRESS,
                        fault_memory_index::ARCHITECTURE_FAULT_CODE>(
                        receiver,
                        1,                                          // is_success
                        0,                                          // error_code
                        info.data,                                  // message_info
                        sender.identifier_when_blocked,             // identifier
                        static_cast<a9n::word>(fault_type::MEMORY), // fault_reason
                        fault_pc,                                   // fault_program_counter
                        sender.fault_address,                       // fault_address
                        sender.arch_fault_code                      // architecture_fault_code
                    );
                    // debug all params
                    DEBUG_LOG("is_success : %d", 1);
                    DEBUG_LOG("error_code : %d", 0);
                    DEBUG_LOG("message_info : 0x%016llx", info.data);
                    DEBUG_LOG("identifier (send) : 0x%016llx", sender.identifier_when_blocked);
                    DEBUG_LOG("identifier (recv) : 0x%016llx", receiver.identifier_when_blocked);
                    DEBUG_LOG("fault_reason : %s", fault_type_to_string(sender.fault_reason));
                    DEBUG_LOG("fault_program_counter : 0x%016llx", fault_pc);
                    DEBUG_LOG("fault_address : 0x%016llx", sender.fault_address);
                    DEBUG_LOG("architecture_fault_code : 0x%016llx", sender.arch_fault_code);
                    break;
                }
            case fault_type::MEMORY_INSTRUCTION_FETCH :
                {
                    auto info = make_message_info(fault_memory_index::MESSAGE_LENGTH);
                    result    = write_message_registers<
                           fault_index::IS_SUCCESS,
                           fault_index::ERROR_CODE,
                           fault_index::MESSAGE_INFO,
                           fault_index::IDENTIFIER,
                           fault_index::FAULT_REASON,
                           fault_memory_index::FAULT_PROGRAM_COUNTER,
                           fault_memory_index::FAULT_ADDRESS,
                           fault_memory_index::ARCHITECTURE_FAULT_CODE>(
                        receiver,
                        1,                              // is_success
                        0,                              // error_code
                        info.data,                      // message_info
                        sender.identifier_when_blocked, // identifier
                        static_cast<a9n::word>(fault_type::MEMORY_INSTRUCTION_FETCH), //
                        fault_pc,              // fault_program_counter
                        sender.fault_address,  // fault_address
                        sender.arch_fault_code // architecture_fault_code
                    );
                    break;
                }
            case fault_type::INVALID_INSTRUCTION :
                {
                    auto info = make_message_info(fault_invalid_instruction_index::MESSAGE_LENGTH);
                    result    = write_message_registers<
                           fault_index::IS_SUCCESS,
                           fault_index::ERROR_CODE,
                           fault_index::MESSAGE_INFO,
                           fault_index::IDENTIFIER,
                           fault_index::FAULT_REASON,
                           fault_invalid_instruction_index::FAULT_PROGRAM_COUNTER,
                           fault_invalid_instruction_index::ARCHITECTURE_FAULT_CODE>(
                        receiver,
                        1,                                                       // is_success
                        0,                                                       // error_code
                        info.data,                                               // message_info
                        sender.identifier_when_blocked,                          // identifier
                        static_cast<a9n::word>(fault_type::INVALID_INSTRUCTION), //
                        fault_pc, // fault_program_counter
                        sender.arch_fault_code
                    );
                    break;
                }
            case fault_type::INVALID_ARITHMETIC :
                {
                    auto info = make_message_info(fault_invalid_arithmetic_index::MESSAGE_LENGTH);
                    result    = write_message_registers<
                           fault_index::IS_SUCCESS,
                           fault_index::ERROR_CODE,
                           fault_index::MESSAGE_INFO,
                           fault_index::IDENTIFIER,
                           fault_index::FAULT_REASON,
                           fault_invalid_arithmetic_index::FAULT_PROGRAM_COUNTER,
                           fault_invalid_arithmetic_index::ARCHITECTURE_FAULT_CODE>(
                        receiver,
                        1,                                                      // is_success
                        0,                                                      // error_code
                        info.data,                                              // message_info
                        sender.identifier_when_blocked,                         // identifier
                        static_cast<a9n::word>(fault_type::INVALID_ARITHMETIC), //
                        fault_pc, // fault_program_counter
                        sender.arch_fault_code
                    );
                    break;
                }
            case fault_type::INVALID_KERNEL_CALL :
                {
                    auto info = make_message_info(fault_invalid_kernel_call_index::MESSAGE_LENGTH);
                    result    = write_message_registers<
                           fault_index::IS_SUCCESS,
                           fault_index::ERROR_CODE,
                           fault_index::MESSAGE_INFO,
                           fault_index::IDENTIFIER,
                           fault_index::FAULT_REASON,
                           fault_invalid_kernel_call_index::FAULT_PROGRAM_COUNTER,
                           fault_invalid_kernel_call_index::KERNEL_CALL_NUMBER>(
                        receiver,
                        1,                                                       // is_success
                        0,                                                       // error_code
                        info.data,                                               // message_info
                        sender.identifier_when_blocked,                          // identifier
                        static_cast<a9n::word>(fault_type::INVALID_KERNEL_CALL), //
                        fault_pc, // fault_program_counter
                        sender.fault_code
                    );
                    break;
                }
            case fault_type::ARCHITECTURE :
                {
                    auto info = make_message_info(fault_architecture_index::MESSAGE_LENGTH);
                    result    = write_message_registers<
                           fault_index::IS_SUCCESS,
                           fault_index::ERROR_CODE,
                           fault_index::MESSAGE_INFO,
                           fault_index::IDENTIFIER,
                           fault_index::FAULT_REASON,
                           fault_architecture_index::FAULT_PROGRAM_COUNTER,
                           fault_architecture_index::ARCHITECTURE_FAULT_CODE>(
                        receiver,
                        1,                                                // is_success
                        0,                                                // error_code
                        info.data,                                        // message_info
                        sender.identifier_when_blocked,                   // identifier
                        static_cast<a9n::word>(fault_type::ARCHITECTURE), // fault_reason
                        fault_pc,                                         // fault_program_counter
                        sender.arch_fault_code // architecture_fault_code; for x86, it's the
                                               // content of the machine check error code register;
                    );
                    break;
                }

            // Unrecoverable error (Fallthrough to Double Fault)
            [[unlikely]] case fault_type::FATAL :
                [[fallthrough]];

            [[unlikely]] default :
                {
                    // unreachable
                    return capability_error::FATAL;
                }
        }

        if (!result) [[unlikely]]
        {
            DEBUG_LOG("failed to write fault message registers");
            return result.unwrap_error();
        }

        auto written_identifier = a9n::hal::get_message_register(receiver, fault_index::IDENTIFIER)
                                      .unwrap_or(static_cast<a9n::word>(0));
        DEBUG_LOG("written identifier : 0x%016llx", written_identifier);

        return {};
    }

    bool ipc_port::is_synchronized(void)
    {
        if (state != WAIT)
        {
            if (!queue_head || !queue_tail) [[unlikely]]
            {
                // synchronization failed;
                DEBUG_LOG("state is not WAIT but queue is empty");
                DEBUG_LOG("queue_head : 0x%016llx", queue_head);
                DEBUG_LOG("queue_tail  : 0x%016llx", queue_tail);

                state = WAIT;
                return false;
            }
        }

        return true;
    }

    // Perform a pure payload copy without any metadata (e.g., message_info, identifier) copy.
    kernel_result ipc_port::copy_messages(
        process  &destination_process,
        process  &source_process,
        a9n::word message_length
    )
    {
        auto configure_value_from_register = [&](a9n::word index) -> a9n::hal::hal_result
        {
            index += (PAYLOAD_START); // skip descriptor, operation_type, message_info, identifier

            DEBUG_LOG("get message register");
            return a9n::hal::get_message_register(source_process, index)
                .and_then(
                    [&](a9n::word v) -> a9n::hal::hal_result
                    {
                        DEBUG_LOG("copy MR[%llu] value=0x%016llx -> MR[%llu]", index, v, index);
                        DEBUG_LOG("configure message register");
                        return a9n::hal::configure_message_register(destination_process, index, v);
                    }
                );
        };

        DEBUG_LOG("message_length : %llu", message_length);
        // min_copy_length = 2 (message_info, identifier)
        for (a9n::word i = 0; i < message_length; i++)
        {
            DEBUG_LOG("copy message : %llu", i);
            auto result = configure_value_from_register(i);
            if (!result) [[unlikely]]
            {
                DEBUG_LOG("failed to copy message : %llu", i);
                DEBUG_LOG("error : %s", hal_error_to_string(result.unwrap_error()));
                return result.transform_error(convert_hal_to_kernel_error);
            }
        }

        return {};
    }

    capability_result ipc_port::move_capabilities(
        process  &destination_process,
        process  &source_process,
        a9n::word transfer_count
    )
    {
        // no transfer
        if (transfer_count == 0) [[likely]]
        {
            return {};
        }

        if ((!destination_process.buffer
             || destination_process.buffer_frame.type != capability_type::FRAME)
            || (!source_process.buffer || source_process.buffer_frame.type != capability_type::FRAME))
            [[unlikely]]
        {
            return capability_error::ILLEGAL_OPERATION;
        }

        return destination_process.root_slot.component
            ->traverse_slot(
                destination_process.buffer->transfer_destination_node,
                extract_depth(destination_process.buffer->transfer_destination_node),
                a9n::BYTE_BITS
            )
            .transform_error(
                []([[maybe_unused]] capability_lookup_error e) -> capability_error
                {
                    return capability_error::INVALID_ARGUMENT;
                }
            )
            .and_then(
                [&](capability_slot *destination_node_slot) -> capability_result
                {
                    if (!destination_node_slot->component
                        || destination_node_slot->type != capability_type::NODE) [[unlikely]]
                    {
                        return capability_error::INVALID_ARGUMENT;
                    }

                    auto offset = destination_process.buffer->transfer_destination_index;

                    for (auto i = 0; i < transfer_count; i++)
                    {
                        auto source_descriptor = source_process.buffer->transfer_source_descriptors[i];
                        auto result
                            = source_process.root_slot.component
                                  ->traverse_slot(
                                      source_descriptor,
                                      extract_depth(source_descriptor),
                                      a9n::BYTE_BITS
                                  )
                                  .transform_error(
                                      []([[maybe_unused]] capability_lookup_error e) -> capability_error
                                      {
                                          return capability_error::INVALID_ARGUMENT;
                                      }
                                  )
                                  .and_then(
                                      [&](capability_slot *source_slot) -> capability_result
                                      {
                                          return destination_node_slot->component
                                              ->retrieve_slot(i + offset)
                                              .transform_error(
                                                  []([[maybe_unused]] capability_lookup_error e) -> capability_error
                                                  {
                                                      return capability_error::INVALID_ARGUMENT;
                                                  }
                                              )
                                              .and_then(
                                                  [&](capability_slot *destination_slot) -> capability_result
                                                  {
                                                      return try_move_capability_slot(
                                                                 *destination_slot,
                                                                 *source_slot
                                                      )
                                                          .transform_error(
                                                              [](kernel_error e) -> capability_error
                                                              {
                                                                  return capability_error::INVALID_ARGUMENT;
                                                              }
                                                          );
                                                  }
                                              );
                                      }
                                  );

                        if (!result) [[unlikely]]
                        {
                            return result.unwrap_error();
                        }
                    }

                    return {};
                }
            );
    }

    kernel_result ipc_port::push_ipc_queue(process &target_process)
    {
        DEBUG_LOG("[PUSH] identifier_when_blocked : 0x%016llx", target_process.identifier_when_blocked);
        target_process.current_ipc_port = this;

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
            target_process.next_ipc_queue    = nullptr;
            target_process.preview_ipc_queue = queue_tail;
            queue_tail->next_ipc_queue       = &target_process;
            queue_tail                       = &target_process;
        }

        DEBUG_LOG("push ipc queue");
        DEBUG_LOG("queue_head : 0x%016llx", queue_head);
        DEBUG_LOG("queue_tail  : 0x%016llx", queue_tail);

        return {};
    }

    liba9n::result<liba9n::not_null<process>, kernel_error> ipc_port::pop_ipc_queue(void)
    {
        // target (queue_head) null check is already done
        auto target = queue_head;
        if (!target) [[unlikely]]
        {
            return kernel_error::NO_SUCH_ADDRESS;
        }

        queue_head = queue_head->next_ipc_queue;

        if (!queue_head)
        {
            queue_tail = nullptr;
            state      = WAIT;
        }
        else
        {
            queue_head->preview_ipc_queue = nullptr;
        }

        target->next_ipc_queue    = nullptr;
        target->preview_ipc_queue = nullptr;
        target->current_ipc_port  = nullptr;

        DEBUG_LOG("pop ipc queue");
        DEBUG_LOG("queue_head : 0x%016llx", queue_head);
        DEBUG_LOG("queue_tail  : 0x%016llx", queue_tail);

        DEBUG_LOG("[POP] identifier_when_blocked : 0x%016llx", target->identifier_when_blocked);

        return liba9n::not_null<process> { *target };
    }
}
