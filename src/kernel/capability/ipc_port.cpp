#include "kernel/types.hpp"
#include <kernel/capability/ipc_port.hpp>

#include <kernel/capability/capability_result.hpp>
#include <kernel/capability/capability_utility.hpp>
#include <kernel/interrupt/fault.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/process/process_manager.hpp>

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
                    // execute(...) is calld only from user.
                    // when called by user, the kernel bit is always 0
                    info.configure_kernel(false);

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
                {
                    if (!info.is_block()) [[unlikely]]
                    {
                        // receiver is not ready
                        return {};
                    }

                    owner.status = process_status::BLOCKED;

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
                {
                    return pop_ipc_queue()
                        .transform_error(convert_kernel_to_capability_error)
                        .and_then(
                            [&](liba9n::not_null<process> target) -> capability_result
                            {
                                target->status = process_status::READY;

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

        switch (state)
        {
            case WAIT :
                DEBUG_LOG("WAIT");
                state = READY_TO_RECEIVE;
                [[fallthrough]];
            case READY_TO_RECEIVE :
                {
                    DEBUG_LOG("READY_TO_RECEIVE");
                    if (!info.is_block()) [[unlikely]]
                    {
                        // receiver is not ready
                        return {};
                    }

                    owner.status = process_status::BLOCKED;

                    return push_ipc_queue(owner)
                        .and_then(try_schedule_and_switch)
                        .transform_error(convert_kernel_to_capability_error);
                }

            case READY_TO_SEND :
                {
                    DEBUG_LOG("READY_TO_SEND");
                    return pop_ipc_queue()
                        .transform_error(convert_kernel_to_capability_error)
                        .and_then(
                            [&](liba9n::not_null<process> target) -> capability_result
                            {
                                auto target_message_info = get_message_info(target.get()).unwrap();
                                return transfer_message(owner, target.get(), target_message_info)
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

    // seL4-derived IPC fastpath implementation
    // TODO: implement fastpath
    capability_result
        ipc_port::operation_call_fastpath(process &owner, capability_slot &self, message_info info)
    {
        // 1. get receiver tcb                  +-----------------+
        // 2. mark sender blocked and enqueue   | no state change |
        // 3. switch to receiver                +-----------------+

        bool slow
            = (!(self.rights & capability_slot::WRITE)) || (state != READY_TO_RECEIVE)
           || !((
               queue_head && (queue_head->reply_state == process::reply_state_object::READY_TO_REPLY)
               || !(owner.priority <= queue_head->priority)
           ));
        if (slow) [[unlikely]]
        {
            return operation_call(owner, self, info);
        }

        // commit state

        // send

        return {};
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
                {
                    if (!info.is_block()) [[unlikely]]
                    {
                        // receiver is not ready
                        return {};
                    }

                    owner.status      = process_status::BLOCKED;
                    owner.reply_state = process::reply_state_object::WAIT;

                    return push_ipc_queue(owner)
                        .and_then(try_schedule_and_switch)
                        .transform_error(convert_kernel_to_capability_error);
                }

            [[likely]] case READY_TO_RECEIVE :
                {
                    return pop_ipc_queue()
                        .transform_error(convert_kernel_to_capability_error)
                        .and_then(
                            [&](liba9n::not_null<process> target) -> capability_result
                            {
                                switch (target->reply_state)
                                {
                                        /*
                                case process::reply_state_object::NONE :
                                    {
                                        target->reply_state
                                            = process::reply_state_object::READY_TO_REPLY;
                                        [[fallthrough]];
                                    }
                                case process::reply_state_object::READY_TO_REPLY :
                                    {
                                        // overwrite
                                        target->reply_target = &owner;

                                        if (!info.is_block()) [[unlikely]]
                                        {
                                            // receiver is not ready
                                            return {};
                                        }

                                        return process_manager_core.try_schedule_and_switch()
                                            .transform_error(convert_kernel_to_capability_error);
                                        [[fallthrough]];
                                    }
                                        */

                                    case process::reply_state_object::NONE :
                                        [[fallthrough]];
                                    case process::reply_state_object::READY_TO_REPLY :
                                        target->reply_state = process::reply_state_object::WAIT;
                                        [[fallthrough]];
                                    [[likely]] case process::reply_state_object::WAIT :
                                        return transfer_message(target.get(), owner, info)
                                            .and_then(
                                                [&, this](void) -> capability_result
                                                {
                                                    target->status = process_status::READY;
                                                    target->reply_state
                                                        = process::reply_state_object::READY_TO_REPLY;
                                                    target->reply_target = &owner;

                                                    return process_manager_core
                                                        .try_direct_schedule_and_switch(target.get())
                                                        .transform_error(convert_kernel_to_capability_error
                                                        );
                                                }
                                            );

                                    [[unlikely]] default :
                                        return capability_error::ILLEGAL_OPERATION;
                                }
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
        switch (owner.reply_state)
        {
            [[likely]] case process::reply_state_object::READY_TO_REPLY :
                {
                    DEBUG_LOG("ready to reply");
                    // available for immediate `reply`
                    if (!owner.reply_target) [[unlikely]]
                    {
                        return capability_error::FATAL;
                    }

                    DEBUG_LOG("reply_target : 0x%016llx", owner.reply_target);
                    auto client = owner.reply_target;

                    return transfer_message(*client, owner, info)
                        .and_then(
                            [&](void) -> capability_result
                            {
                                DEBUG_LOG("initialize reply_target");
                                // initialize
                                client->status       = process_status::READY;
                                client->reply_state  = process::reply_state_object::NONE;
                                client->reply_target = nullptr;

                                owner.reply_state    = process::reply_state_object::NONE;
                                owner.reply_target   = nullptr;

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

            case process::reply_state_object::NONE :
            case process::reply_state_object::WAIT :
                return {};

            [[unlikely]] default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_result
        ipc_port::operation_reply_receive_fastpath(process &owner, capability_slot &self, message_info info)
    {
        return {};
    }

    capability_result
        ipc_port::operation_reply_receive(process &owner, capability_slot &self, message_info info)
    {
        return operation_reply(owner, self, info)
            .and_then(
                [&, this](void) -> capability_result
                {
                    return operation_receive(owner, self, info);
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

        return a9n::hal::get_message_register(owner, IDENTIFIER)
            .transform_error(convert_hal_to_kernel_error)
            .transform_error(convert_kernel_to_capability_error)
            .and_then(
                [&self](a9n::word identifier) -> capability_result
                {
                    DEBUG_LOG("identifier : 0x%16llx to slot 0x%016llx", identifier, &self);
                    self.data = convert_identifier_to_slot_data(identifier);

                    return {};
                }
            );
    }

    // TODO: implement fault handling
    capability_result ipc_port::operation_fault_call(process &owner, capability_slot &self)
    {
        if (!(self.rights & capability_slot::WRITE) || !(self.rights & capability_slot::READ))
            [[unlikely]]
        {
            owner.status = process_status::BLOCKED;
            return capability_error::PERMISSION_DENIED;
        }

        message_info info {
            true, // block
            0,    // message length
            0,    // transfer count
            true  // kernel
        };

        switch (owner.fault_reason)
        {
            case fault_type::MEMORY :
                {
                    return operation_call(owner, self, info);
                }

            case fault_type::INVALID_INSTRUCTION :
                {
                }

            case fault_type::INVALID_ARITHMETIC :
                {
                    return operation_call(owner, self, info);
                }

            case fault_type::INVALID_KERNEL_CALL :
                {
                    return operation_call(owner, self, info);
                }

            case fault_type::ARCHITECTURE :
                {
                    return operation_call(owner, self, info);
                }

            case fault_type::FATAL :
                {
                    return operation_call(owner, self, info);
                }

            [[unlikely]] default :
                {
                    return operation_call(owner, self, info);
                }
        }
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
                        "transfer_count : %u",
                        target_message_info.is_block() ? 'T' : 'F',
                        target_message_info.message_length(),
                        target_message_info.transfer_count()
                    );

                    return target_message_info;
                }
            );
    }

    capability_result ipc_port::transfer_message(process &receiver, process &sender, message_info info)
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

    bool ipc_port::is_synchronized(void)
    {
        if (state != WAIT)
        {
            if (!queue_head || !queue_end) [[unlikely]]
            {
                // synchronization failed;
                DEBUG_LOG("state is not WAIT but queue is empty");
                DEBUG_LOG("queue_head : 0x%016llx", queue_head);
                DEBUG_LOG("queue_end  : 0x%016llx", queue_end);

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
            index += (MESSAGE_INFO); // skip descriptor, operation_type

            DEBUG_LOG("get message register");
            return a9n::hal::get_message_register(source_process, index)
                .and_then(
                    [&](a9n::word v) -> a9n::hal::hal_result
                    {
                        DEBUG_LOG("configure message register");
                        return a9n::hal::configure_message_register(destination_process, index, v);
                    }
                );
        };

        DEBUG_LOG("message_length : %llu", message_length);
        // min_copy_length = 2 (message_info, identifier)
        for (a9n::word i = 0; i < message_length + 2; i++)
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
                                                  []([[maybe_unused]] capability_lookup_error e
                                                  ) -> capability_error
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

        DEBUG_LOG("push ipc queue");
        DEBUG_LOG("queue_head : 0x%016llx", queue_head);
        DEBUG_LOG("queue_end  : 0x%016llx", queue_end);

        return {};
    }

    liba9n::result<liba9n::not_null<process>, kernel_error> ipc_port::pop_ipc_queue(void)
    {
        // target (queue_head) null check is already done
        auto target = queue_head;
        queue_head  = queue_head->next_ipc_queue;

        if (!queue_head)
        {
            queue_end = nullptr;
            state     = WAIT;
        }

        if (!target) [[unlikely]]
        {
            return kernel_error::NO_SUCH_ADDRESS;
        }

        target->next_ipc_queue    = nullptr;
        target->preview_ipc_queue = nullptr;

        // TEST
        queue_head->preview_ipc_queue = nullptr;

        DEBUG_LOG("pop ipc queue");
        DEBUG_LOG("queue_head : 0x%016llx", queue_head);
        DEBUG_LOG("queue_end  : 0x%016llx", queue_end);

        return liba9n::not_null<process> { *target };
    }
}
