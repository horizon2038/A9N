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
    class ipc_port;
    class notification_port;

    class ipc_port : public capability_component
    {
      private:
        // head / end makes the search O(1)
        process *queue_head { nullptr };
        process *queue_tail { nullptr }; // TODO: rename to queue_tail

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
            IDENTIFIER_SOURCE = MESSAGE_INFO, // for identify()
            IDENTIFIER_DESTINATION,           // for send/receive/call/reply/reply_receive
            PAYLOAD_START,
        };

        struct message_info
        {
            enum class message_source : uint8_t
            {
                NORMAL       = 0,
                FAULT        = 1,
                NOTIFICATION = 2,
                RESERVED     = 3,
            };

            a9n::word data;

            static constexpr a9n::word BLOCK_SHIFT          = 0;
            static constexpr a9n::word MESSAGE_LENGTH_SHIFT = 1;
            static constexpr a9n::word TRANSFER_COUNT_SHIFT = 9;
            static constexpr a9n::word SOURCE_SHIFT         = 13;
            static constexpr a9n::word RESERVED_SHIFT       = 15;

            static constexpr a9n::word BLOCK_MASK = static_cast<a9n::word>(0x01) << BLOCK_SHIFT;
            static constexpr a9n::word MESSAGE_LENGTH_MASK
                = static_cast<a9n::word>(0xFF) << MESSAGE_LENGTH_SHIFT;
            static constexpr a9n::word TRANSFER_COUNT_MASK
                = static_cast<a9n::word>(0x0F) << TRANSFER_COUNT_SHIFT;
            static constexpr a9n::word SOURCE_MASK = static_cast<a9n::word>(0x03) << SOURCE_SHIFT;
            static constexpr a9n::word RESERVED_MASK = static_cast<a9n::word>(0x01) << RESERVED_SHIFT;

            constexpr message_info(
                bool           block,
                uint8_t        message_length,
                uint8_t        transfer_count,
                message_source source
            )
                : data(0)
            {
                configure_block(block);
                configure_message_length(message_length);
                configure_transfer_count(transfer_count);
                configure_source(source);
            }

            constexpr explicit message_info(a9n::word initial_data) : data(initial_data)
            {
            }

            constexpr void configure_block(bool is_block)
            {
                data = (data & ~BLOCK_MASK) | (((a9n::word)is_block) << BLOCK_SHIFT);
            }

            constexpr void configure_message_length(uint8_t new_message_length)
            {
                data = (data & ~MESSAGE_LENGTH_MASK)
                     | ((((a9n::word)new_message_length) & 0xFF) << MESSAGE_LENGTH_SHIFT);
            }

            constexpr void configure_transfer_count(uint8_t new_transfer_count)
            {
                data = (data & ~TRANSFER_COUNT_MASK)
                     | ((static_cast<a9n::word>(new_transfer_count) & 0x0f) << TRANSFER_COUNT_SHIFT);
            }

            constexpr void configure_source(message_source new_source)
            {
                data = (data & ~SOURCE_MASK)
                     | ((static_cast<a9n::word>(new_source) & 0x03) << SOURCE_SHIFT);
            }

            constexpr bool is_block(void) const
            {
                return ((data & BLOCK_MASK) >> BLOCK_SHIFT) != 0;
            }

            constexpr uint8_t message_length(void) const
            {
                return (uint8_t)((data & MESSAGE_LENGTH_MASK) >> MESSAGE_LENGTH_SHIFT);
            }

            constexpr uint8_t transfer_count(void) const
            {
                return (uint8_t)((data & TRANSFER_COUNT_MASK) >> TRANSFER_COUNT_SHIFT);
            }

            constexpr message_source source(void) const
            {
                return static_cast<message_source>((data & SOURCE_MASK) >> SOURCE_SHIFT);
            }

            constexpr bool is_normal(void) const
            {
                return source() == message_source::NORMAL;
            }

            constexpr bool is_fault(void) const
            {
                return source() == message_source::FAULT;
            }

            constexpr bool is_notification(void) const
            {
                return source() == message_source::NOTIFICATION;
            }
        };

        // capability-call
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

      private:
        // internal functions
        liba9n::result<message_info, kernel_error> get_message_info(process &owner);
        capability_result transfer_message(process &receiver, process &sender, message_info info);
        capability_result
            transfer_direct_message(process &receiver, process &sender, message_info info);
        capability_result transfer_fault_message(process &receiver, process &sender);
        capability_result
            complete_fault_reply_without_message_transfer(process &owner, process &client);
        capability_result complete_reply_without_switch(process &owner, message_info info);
        capability_result try_receive_from_ready_sender(process &receiver);
        capability_result try_deliver_pending_binded_notification(process &owner, bool &delivered);

        bool is_synchronized(void);
        kernel_result
            copy_messages(process &destination_process, process &source_process, a9n::word message_length);
        capability_result move_capabilities(
            process  &destination_process,
            process  &source_process,
            a9n::word transfer_count
        );

        kernel_result push_ipc_queue(process &target_process);
        liba9n::result<liba9n::not_null<process>, kernel_error> pop_ipc_queue(void);
        // for notification port
        kernel_result remove_ipc_queue(process &target_process);

        // capability management
      public:
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

      private:
        friend class notification_port;
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
