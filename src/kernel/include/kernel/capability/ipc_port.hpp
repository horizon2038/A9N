#ifndef A9N_KERNEL_DRAFT_IPC_PORT_HPP
#define A9N_KERNEL_DRAFT_IPC_PORT_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_result.hpp>
#include <kernel/capability/operation/ipc_port_operation.hpp>
#include <kernel/process/process.hpp>

namespace a9n::kernel
{
    enum class ipc_port_state : a9n::word
    {
        WAIT,
        READY_TO_SEND,
        READY_TO_RECEIVE,
    };

    // 64-byte align !
    // now, already used 8 byte (v-table cost)
    class ipc_port : capability_component
    {
      private:
        // head / end makes the search O(1)
        process       *queue_head;
        process       *queue_end;
        ipc_port_state state { ipc_port_state::WAIT };

      public:
        capability_result
            execute(process &this_process, capability_slot &this_slot) override
        {
            using enum ipc_port_operation;

            capability_result result;
            switch (static_cast<ipc_port_operation>(this_process.buffer->message_tag
            ))
            {
                case SEND :
                    result = operation_send(this_process, this_slot);
                    break;

                case RECEIVE :
                    result = operation_receive(this_process, this_slot);
                    break;

                case CALL :
                    break;

                case REPLY :
                    break;

                default :
                    return capability_error::ILLEGAL_OPERATION;
            }

            return {};
        }

        capability_result
            operation_send(process &this_process, capability_slot &this_slot)

        {
            using enum ipc_port_state;

            bool is_block = static_cast<bool>(
                this_process.buffer->get_message<ipc_port_send_argument::IS_BLOCK>()
            );

            // send message
            switch (state)
            {
                case WAIT :
                    {
                        state      = READY_TO_RECEIVE;
                        queue_head = &this_process;
                        queue_end  = &this_process;

                        if (is_block)
                        {
                            this_process.status = process_status::BLOCKED;
                        }

                        break;
                    }

                case READY_TO_RECEIVE :
                    {
                        state       = WAIT;

                        auto target = queue_head;
                        while (target)
                        {
                            liba9n::std::memcpy(
                                target->buffer,
                                this_process.buffer,
                                target->buffer->message_size
                            );

                            target->status = process_status::READY;
                            target         = target->next_ipc_queue;
                        }

                        break;
                    }

                case READY_TO_SEND :
                    // TODO: determines behaviour when senders collide
                    {
                        [[unlikely]]
                        if (!queue_end)
                        {
                            // sync failed
                            return capability_error::FATAL;
                        }

                        queue_end->next_ipc_queue = &this_process;
                        queue_end                 = &this_process;

                        break;
                    }

                default :
                    return capability_error::ILLEGAL_OPERATION;
            }

            return {};
        }

        capability_result
            operation_receive(process &this_process, capability_slot &this_slot)
        {
            using enum ipc_port_state;

            bool is_block = static_cast<bool>(
                this_process.buffer->get_message<ipc_port_send_argument::IS_BLOCK>()
            );

            switch (state)
            {
                case WAIT :
                    {
                        state      = READY_TO_RECEIVE;
                        queue_head = &this_process;
                        queue_end  = &this_process;

                        if (is_block)
                        {
                            this_process.status = process_status::BLOCKED;
                        }

                        break;
                    }

                case READY_TO_RECEIVE :
                    {
                        queue_head = &this_process;
                        if (!queue_end)
                        {
                            return capability_error::FATAL;
                        }

                        queue_end->next_ipc_queue = &this_process;
                        queue_end                 = &this_process;

                        break;
                    }

                case READY_TO_SEND :
                    {
                        state       = WAIT;

                        auto target = queue_head;
                        while (target)
                        {
                            liba9n::std::memcpy(
                                this_process.buffer,
                                target->buffer,
                                target->buffer->message_size
                            );

                            target->status = process_status::READY;
                            target         = target->next_ipc_queue;
                        }

                        break;
                    }

                default :
                    return capability_error::ILLEGAL_OPERATION;
            }

            return {};
        }

        capability_result
            operation_call(process &this_process, capability_slot &this_slot)
        {
            auto result
                = operation_send(this_process, this_slot)
                      .and_then(
                          [&, this](void) -> capability_result
                          {
                              return operation_receive(this_process, this_slot);
                          }
                      );

            return result;
        }

        capability_lookup_result retrieve_slot(a9n::word index) override
        {
            return capability_lookup_error::TERMINAL;
        }

        capability_lookup_result traverse_slot(
            a9n::capability_descriptor descriptor,
            a9n::word                  max_bits,
            a9n::word                  used_bits
        ) override
        {
            return capability_lookup_error::TERMINAL;
        };
    };
}

#endif
