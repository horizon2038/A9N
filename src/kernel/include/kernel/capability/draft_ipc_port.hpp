#ifndef A9N_KERNEL_DRAFT_IPC_PORT_HPP
#define A9N_KERNEL_DRAFT_IPC_PORT_HPP

#include "kernel/capability/capability_result.hpp"
#include <kernel/capability/capability_component.hpp>
#include <kernel/process/process.hpp>

namespace a9n::kernel
{
    namespace ipc_port_operation
    {
    }

    enum class draft_ipc_port_operation : word
    {
        SEND,
        RECEIVE,
        CALL,
        REPLY,
    };

    namespace draft_ipc_port_send_argument
    {
        inline constexpr bool IS_BLOCK      = 0;
        inline constexpr bool MESSAGE_COUNT = 1;
    }

    enum class draft_ipc_port_state : a9n::word
    {
        WAIT,
        READY_TO_SEND,
        READY_TO_RECEIVE,
    };

    // 64-byte align !
    // now, already used 8 byte (v-table cost)
    class draft_ipc_port
    {
      private:
        // head / end makes the search O(1)
        v2::process         *queue_head;
        v2::process         *queue_end;
        draft_ipc_port_state state { draft_ipc_port_state::WAIT };

      public:
        capability_result
            execute(ipc_buffer *buffer, capability_slot *this_slot, v2::process)
        {
            using enum draft_ipc_port_operation;

            switch (static_cast<draft_ipc_port_operation>(buffer->message_tag))
            {
                case SEND :

                case RECEIVE :
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

        capability_result operation_send(
            const ipc_buffer &buffer,
            capability_slot  &this_slot,
            v2::process      &this_process
        )

        {
            using enum draft_ipc_port_state;

            bool is_block
                = buffer.get_message<draft_ipc_port_send_argument::IS_BLOCK>();

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

        capability_result operation_receive(
            const ipc_buffer &buffer,
            capability_slot  &this_slot,
            v2::process      &this_process
        )
        {
            using enum draft_ipc_port_state;

            bool is_block
                = buffer.get_message<draft_ipc_port_send_argument::IS_BLOCK>();

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

        capability_result operation_call(
            const ipc_buffer &buffer,
            capability_slot  &this_slot,
            v2::process       this_process
        )
        {
            auto result
                = operation_send(buffer, this_slot, this_process)
                      .and_then(
                          [&, this](void) -> capability_result
                          {
                              operation_receive(buffer, this_slot, this_process);
                              return {};
                          }
                      );

            return {};
        }
    };
}

#endif
