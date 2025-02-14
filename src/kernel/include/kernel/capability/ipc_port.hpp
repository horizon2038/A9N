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
    class ipc_port : public capability_component
    {
      private:
        // head / end makes the search O(1)
        process *queue_head;
        process *queue_end; // TODO: rename to queue_tail

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
            REPLY_RECEIVE, // use reply object
            IDENTIFY,
        };

        enum operation_index : a9n::word
        {
            RESERVED = 0, // descriptor
            OPERATION_TYPE,
            MESSAGE_INFO,
            IDENTIFIER,
        };

        // TODO: add "kernel message" flag
        struct message_info
        {
            a9n::word data; // 16 bits

            constexpr message_info(
                bool    block,
                uint8_t message_length,
                uint8_t transfer_count,
                bool    kernel // if the user sets the kernel bit, the value is ignored
            )
            {
                configure_block(block);
                configure_message_length(message_length);
                configure_transfer_count(transfer_count);
                configure_kernel(kernel);
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
                data = data & ~(((1 << 8) - 1) << 1) | ((message_length << 1) & ((1 << 8) - 1));
            }

            constexpr void configure_transfer_count(uint8_t transfer_count)
            {
                data = data & ~(((1 << 5) - 1) << 9) | ((transfer_count << 9) & (1 << 6) - 1);
            }

            constexpr void configure_kernel(bool kernel)
            {
                data = data & ~(1 << 15) | (kernel << 15);
            }

            constexpr bool is_block(void) const
            {
                return data & (1 << 0);
            }

            constexpr uint8_t message_length(void) const
            {
                return (data >> 1) & ((1 << 8) - 1);
            }

            constexpr uint8_t transfer_count(void) const
            {
                return (data >> 9) & ((1 << 6) - 1);
            }

            constexpr bool is_kernel(void) const
            {
                return data & (1 << 15);
            }
        };

      public:
        capability_result execute(process &owner, capability_slot &self) override;

        capability_result operation_send(process &owner, capability_slot &self, message_info info);
        capability_result operation_receive(process &owner, capability_slot &self, message_info info);
        capability_result operation_call(process &owner, capability_slot &self, message_info info);
        capability_result operation_reply(process &owner, capability_slot &self, message_info info);
        capability_result
            operation_reply_receive(process &owner, capability_slot &self, message_info info);
        capability_result operation_identify(process &owner, capability_slot &self);

        // for kernel
        capability_result operation_fault_call(process &owner, capability_slot &self);

        liba9n::result<message_info, kernel_error> get_message_info(process &owner);
        capability_result transfer_message(process &receiver, process &sender, message_info info);
        bool              is_synchronized(void);
        kernel_result
            copy_messages(process &destination_process, process &source_process, a9n::word message_length);
        capability_result move_capabilities(
            process  &destination_process,
            process  &source_process,
            a9n::word transfer_count
        );

        kernel_result push_ipc_queue(process &target_process);
        liba9n::result<liba9n::not_null<process>, kernel_error> pop_ipc_queue(void);

        capability_result revoke(capability_slot &self) override
        {
            return {};
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

    inline constexpr a9n::word convert_slot_data_to_identifier(const capability_slot_data &data)
    {
        return data[0];
    }

    inline constexpr capability_slot_data convert_identifier_to_slot_data(a9n::word identifier)
    {
        capability_slot_data data;
        data[0] = identifier;

        return data;
    }

    inline kernel_result
        try_configure_ipc_port_slot(capability_slot &slot, ipc_port &port, a9n::word identifier)
    {
        slot.init();
        slot.component = &port;
        slot.type      = capability_type::IPC_PORT;
        slot.rights    = capability_slot::ALL;
        slot.data.fill(0);
        slot.data = convert_identifier_to_slot_data(identifier);

        return {};
    }

}

#endif
