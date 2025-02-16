#ifndef A9N_KERNEL_INTERRUPT_PORT_HPP
#define A9N_KERNEL_INTERRUPT_PORT_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/notification_port.hpp>

namespace a9n::kernel
{
    class interrupt_port : public capability_component
    {
      private:
        enum operation_index : a9n::word
        {
            INDEX_RESERVED = 0, // descriptor
            INDEX_OPERATION_TYPE,
            INDEX_NOTIFICATION_PORT_DESCRIPTOR,
        };

        enum operation_type : a9n::word
        {
            OPERATION_NONE,
            OPERATION_BIND_NOTIFICATION_PORT,
            OPERATION_UNBIND_NOTIFICATION_PORT,
            OPERATION_ACK,
            OPERATION_GET_IRQ_NUMBER,
        };

        enum result_index : a9n::word
        {
            RESULT_IS_SUCCESS = 0, // reserved
            RESULT_ERROR_CODE,     // reserved

            // for GET_IRQ_NUMBER
            RESULT_IRQ_NUMBER,
        };

      public:
        capability_result execute(process &owner, capability_slot &self) override;

        capability_result operation_bind_notification_port(process &owner, capability_slot &self);
        capability_result operation_unbind_notification_port(process &owner, capability_slot &self);
        capability_result operation_ack(process &owner, capability_slot &self);
        capability_result operation_get_irq_number(process &owner, capability_slot &self);

        capability_result revoke(capability_slot &self) override;

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
        };
    };

    struct interrupt_port_info
    {
        a9n::word          irq_number;
        notification_port *binded_notification_port;
    };

    inline interrupt_port_info
        convert_slot_data_to_interrupt_port_info(const capability_slot_data &data)
    {
        return {
            .irq_number = data[0],
            // NOTE: unsafe cast
            .binded_notification_port = reinterpret_cast<notification_port *>(data[1]),
        };
    }

    inline capability_slot_data
        convert_interrupt_port_info_to_slot_data(const interrupt_port_info &info)
    {
        capability_slot_data data;

        data[0] = info.irq_number;
        data[1] = reinterpret_cast<a9n::word>(info.binded_notification_port);

        return data;
    }

    inline kernel_result
        try_configure_interrupt_port_slot(capability_slot &slot, interrupt_port &port, a9n::word irq_number)
    {
        slot.init();
        slot.component = &port;
        slot.type      = capability_type::INTERRUPT_PORT;
        slot.rights    = capability_slot::ALL;
        slot.data.fill(0);
        slot.data = convert_interrupt_port_info_to_slot_data({ .irq_number = irq_number });

        return {};
    }
}

#endif
