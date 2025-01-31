#ifndef A9N_KERNEL_NOTIFICATION_PORT_HPP
#define A9N_KERNEL_NOTIFICATION_PORT_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_result.hpp>
#include <kernel/capability/ipc_port.hpp>
#include <kernel/ipc/notification.hpp>
#include <kernel/kernel_result.hpp>

namespace a9n::kernel
{
    class notification_port : public capability_component
    {
      private:
        enum operation_index : a9n::word
        {
            RESERVED = 0, // descriptor
            OPERATION_TYPE,

            RESERVED_MR_2,

            // for IDENTIFY
            NEW_IDENTIFIER = RESERVED_MR_2,
        };

        enum result_index : a9n::word
        {
            IS_SUCCESS,
            ERROR_CODE,

            // for NOTIFY

            // for WAIT/POLL
            FLAG_WORD,
        };

        // "notification" Mechanism
        enum operation_type : a9n::word
        {
            OPERATION_NONE,
            OPERATION_NOTIFY, // send notification bit
            OPERATION_WAIT,   // synchronous
            OPERATION_POLL,   // asynchronous
            OPERATION_IDENTIFY,
        };

        notification core;

        // waitqueue
        process *queue_head;
        process *queue_tail;

        enum notification_port_state : a9n::word
        {
            WAIT = 0,
            READY_TO_WAKE,
        } state { notification_port_state::WAIT };

      public:
        capability_result execute(process &this_process, capability_slot &this_slot) override;

        capability_result operation_notify([[maybe_unused]] process &owner, capability_slot &self);
        capability_result operation_wait(process &owner, capability_slot &self);
        capability_result operation_poll(process &owner, capability_slot &self);
        capability_result operation_identify(process &owner, capability_slot &self);

        // queue control
        kernel_result push_notification_queue(process &target_process);
        liba9n::result<liba9n::not_null<process>, kernel_error> pop_notification_queue(void);

        capability_result revoke(capability_slot &self) override
        {
            // remove all processes from the queue
            return {};
        }

        capability_lookup_result retrieve_slot(a9n::word index) override
        {
            return capability_lookup_error::TERMINAL;
        }

        capability_lookup_result traverse_slot(
            a9n::capability_descriptor descriptor,
            a9n::word                  descriptor_max_bits,
            a9n::word                  descriptor_used_bits
        ) override
        {
            return capability_lookup_error::TERMINAL;
        }
    };

    inline constexpr kernel_result try_configure_notification_port_slot(
        capability_slot   &slot,
        notification_port &port,
        a9n::word          identifier
    )
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
