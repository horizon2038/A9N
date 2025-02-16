#include <kernel/capability/interrupt_port.hpp>

#include <kernel/utility/logger.hpp>

namespace a9n::kernel
{
    namespace
    {
        inline constexpr capability_error
            convert_hal_to_capability_error([[maybe_unused]] a9n::hal::hal_error e)
        {
            return capability_error::FATAL;
        }

        inline capability_error
            convert_lookup_to_capability_error([[maybe_unused]] capability_lookup_error e)
        {
            return capability_error::INVALID_DESCRIPTOR;
        };

    }

    capability_result interrupt_port::execute(process &owner, capability_slot &self)
    {
        DEBUG_LOG("ipc_port::execute");
        auto target_operation = [&]() -> operation_type
        {
            return static_cast<operation_type>(
                a9n::hal::get_message_register(owner, INDEX_OPERATION_TYPE)
                    .unwrap_or(static_cast<a9n::word>(0))
            );
        };

        switch (target_operation())
        {
            case OPERATION_BIND_NOTIFICATION_PORT :
                DEBUG_LOG("interrupt_port::bind_notification_port");
                return operation_bind_notification_port(owner, self);

            case OPERATION_UNBIND_NOTIFICATION_PORT :
                DEBUG_LOG("interrupt_port::unbind_notification_port");
                return operation_unbind_notification_port(owner, self);

            case OPERATION_ACK :
                DEBUG_LOG("interrupt_port::ack");
                return operation_ack(owner, self);

            case OPERATION_GET_IRQ_NUMBER :
                DEBUG_LOG("interrupt_port::get_irq_number");
                return operation_get_irq_number(owner, self);

            default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_result
        interrupt_port::operation_bind_notification_port(process &owner, capability_slot &self)
    {
        return a9n::hal::get_message_register(owner, INDEX_NOTIFICATION_PORT_DESCRIPTOR)
            .transform_error(convert_hal_to_capability_error)
            .and_then(
                [&](a9n::word descriptor) -> capability_result
                {
                    return owner.root_slot.component
                        ->traverse_slot(descriptor, extract_depth(descriptor), a9n::BYTE_BITS)
                        .transform_error(convert_lookup_to_capability_error)
                        .and_then(
                            [&](capability_slot *notification_port_slot) -> capability_result
                            {
                                if (notification_port_slot->type != capability_type::NOTIFICATION_PORT)
                                {
                                    return capability_error::INVALID_DESCRIPTOR;
                                }

                                auto info = convert_slot_data_to_interrupt_port_info(self.data);
                                auto notification_port = static_cast<class notification_port *>(
                                    notification_port_slot->component
                                );

                                if (info.binded_notification_port)
                                {
                                    info.binded_notification_port->binded_interrupt_port = nullptr;
                                }
                                {
                                    return capability_error::INVALID_DESCRIPTOR;
                                }

                                auto port_info = convert_slot_data_to_interrupt_port_info(self.data);
                                if (port_info.binded_notification_port)
                                {
                                    return capability_error::ILLEGAL_OPERATION;
                                }

                                port_info.binded_notification_port       = notification_port;
                                notification_port->binded_interrupt_port = this;

                                self.data = convert_interrupt_port_info_to_slot_data(port_info);

                                return {};
                            }
                        );
                }
            );
    }

    capability_result interrupt_port::revoke(capability_slot &self)
    {
        auto port_info = convert_slot_data_to_interrupt_port_info(self.data);
        if (port_info.binded_notification_port)
        {
            // unbind (remove) self
            port_info.binded_notification_port->binded_interrupt_port = nullptr;
        }

        return {};
    }
}
