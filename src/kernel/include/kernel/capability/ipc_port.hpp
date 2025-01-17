#ifndef A9N_KERNEL_DRAFT_IPC_PORT_HPP
#define A9N_KERNEL_DRAFT_IPC_PORT_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_result.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/process/process.hpp>

#include <hal/hal_result.hpp>
#include <hal/interface/process_manager.hpp>

#include <liba9n/common/not_null.hpp>

namespace a9n::kernel
{
    inline capability_error convert_kernel_to_capability_error(kernel_error e)
    {
        return capability_error::FATAL;
    }

    class ipc_port : capability_component
    {
      private:
        // head / end makes the search O(1)
        process *queue_head;
        process *queue_end;

        enum ipc_port_state : a9n::word
        {
            WAIT = 0,
            READY_TO_SEND,
            READY_TO_RECEIVE,
        } state { ipc_port_state::WAIT };

        enum operation_type : a9n::word
        {
            TYPE_NONE = 0,
            SEND,
            RECEIVE,
            CALL, // use reply object
            REPLY,
            REPLY_RECEIVE // use reply object
        };

        enum operation_index : a9n::word
        {
            RESERVED = 0, // descriptor
            OPERATION_TYPE,
            MESSAGE_INFO,
            TAG,
        };

        struct message_info
        {
            a9n::word data;

            constexpr message_info(
                bool    block,
                uint8_t message_length,
                bool    transfer_capability,
                uint8_t transfer_count
            )
            {
                configure_block(block);
                configure_message_length(message_length);
                configure_transfer_capability(transfer_capability);
                configure_transfer_count(transfer_count);
            }

            constexpr message_info(a9n::word data) : data(data)
            {
            }

            // 0 : block
            // 1-8 : message length
            // 9 : transfer capability_slot
            // 10-15 : transfer count

            constexpr void configure_block(bool is_block)
            {
                data = (data & ~(1 << 0)) | is_block;
            }

            constexpr void configure_message_length(uint8_t message_length)
            {
                data = data & ~(((1 << 8) - 1) << 1) | (message_length << 1);
            }

            constexpr void configure_transfer_capability(bool transfer_capability)
            {
                data = data & ~(1 << 9) | (transfer_capability << 9);
            }

            constexpr void configure_transfer_count(uint8_t transfer_count)
            {
                data = data & ~(((1 << 6) - 1) << 10) | (transfer_count << 10);
            }

            constexpr bool is_block(void) const
            {
                return data & (1 << 0);
            }

            constexpr uint8_t message_length(void) const
            {
                return (data >> 1) & ((1 << 8) - 1);
            }

            constexpr bool is_transfer_capability(void) const
            {
                return data & (1 << 9);
            }

            constexpr uint8_t transfer_count(void) const
            {
                return (data >> 10) & ((1 << 6) - 1);
            }
        };

      public:
        capability_result execute(process &owner, capability_slot &self) override;

        capability_result operation_send(process &owner, capability_slot &self, message_info info);
        capability_result operation_receive(process &owner, capability_slot &self, message_info info);
        capability_result operation_call(process &owner, capability_slot &self, message_info info);
        capability_result operation_reply(process &owner, capability_slot &self, message_info info);
        capability_result
            operation_reply_receive(process &owner, capability_slot &self, message_info &info);

        liba9n::result<message_info, kernel_error> get_message_info(process &owner);
        capability_result transfer_message(process &receiver, process &sender, message_info info);
        bool              is_synchronized(void);
        kernel_result
            copy_messages(process &destination_process, process &source_process, a9n::word message_length);

        kernel_result push_ipc_queue(process &target_process);
        liba9n::result<liba9n::not_null<process>, kernel_error> pop_ipc_queue(void);

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
