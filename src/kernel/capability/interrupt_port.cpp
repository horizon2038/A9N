#include <kernel/capability/interrupt_port.hpp>

#include <hal/interface/interrupt.hpp>
#include <kernel/capability/capability_result.hpp>
#include <kernel/interrupt/interrupt_manager.hpp>
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

                                auto irq_handler_result
                                    = irq_number_to_irq_notification_handler(info.irq_number);

                                if (!irq_handler_result)
                                {
                                    return convert_kernel_to_capability_error(
                                        irq_handler_result.unwrap_error()
                                    );
                                }
                                auto &irq_handler = irq_handler_result.unwrap();

                                if (irq_handler->slot.type != capability_type::NONE)
                                {
                                    // already binded
                                    return capability_error::ILLEGAL_OPERATION;
                                }

                                return try_copy_capability_slot(irq_handler->slot, *notification_port_slot)
                                    .transform_error(convert_kernel_to_capability_error);
                            }
                        );
                }
            );
    }

    capability_result
        interrupt_port::operation_unbind_notification_port(process &owner, capability_slot &self)
    {
        auto port_info      = convert_slot_data_to_interrupt_port_info(self.data);
        auto handler_result = irq_number_to_irq_notification_handler(port_info.irq_number);

        if (!handler_result)
        {
            return convert_kernel_to_capability_error(handler_result.unwrap_error());
        }

        auto &irq_handler = handler_result.unwrap();
        if (irq_handler->slot.type == capability_type::NONE)
        {
            // already unbinded; do nothing
            return {};
        }

        return irq_handler->slot.try_remove_and_init().transform_error(
            convert_kernel_to_capability_error
        );
    }

    capability_result interrupt_port::operation_ack(process &owner, capability_slot &self)
    {
        auto port_info = convert_slot_data_to_interrupt_port_info(self.data);
        DEBUG_LOG("Ack IRQ: %04llu\n", port_info.irq_number);
        auto handler_result = irq_number_to_irq_notification_handler(port_info.irq_number);
        if (!handler_result)
        {
            return convert_kernel_to_capability_error(handler_result.unwrap_error());
        }

        return a9n::kernel::interrupt_manager_core.enable_interrupt(port_info.irq_number)
            .transform_error(convert_kernel_to_capability_error);
    }

    capability_result interrupt_port::operation_get_irq_number(process &owner, capability_slot &self)
    {
        auto port_info = convert_slot_data_to_interrupt_port_info(self.data);

        return a9n::hal::configure_message_register(owner, RESULT_IRQ_NUMBER, port_info.irq_number)
            .transform_error(convert_hal_to_capability_error);
    }

    capability_result interrupt_port::revoke(capability_slot &self)
    {
        auto port_info = convert_slot_data_to_interrupt_port_info(self.data);

        return irq_number_to_irq_notification_handler(port_info.irq_number)
            .transform_error(convert_kernel_to_capability_error)
            .and_then(
                [&](liba9n::not_null<irq_notification_handler> handler) -> capability_result
                {
                    handler->used = false;
                    return handler->slot.try_remove_and_init().transform_error(
                        convert_kernel_to_capability_error
                    );
                }
            );

        return {};
    }
}
