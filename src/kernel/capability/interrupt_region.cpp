#include <kernel/capability/interrupt_region.hpp>

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_invocation.hpp>
#include <kernel/capability/interrupt_port.hpp>
#include <kernel/interrupt/irq_notification_handlers.hpp>
#include <kernel/utility/logger.hpp>

#include <hal/interface/process_manager.hpp>

namespace a9n::kernel
{
    namespace
    {
        inline constexpr capability_error
            convert_hal_to_capability_error([[maybe_unused]] a9n::hal::hal_error e)
        {
            return capability_error::FATAL;
        }

        inline constexpr capability_error convert_lookup_error(capability_lookup_error e)
        {
            return capability_error::INVALID_DESCRIPTOR;
        }
    }

    capability_result interrupt_region::execute(process &owner, capability_slot &self)
    {
        auto operation = static_cast<operation_type>(
            a9n::hal::get_message_register(owner, OPERATION_TYPE)
                .unwrap_or(static_cast<a9n::word>(OPERATION_NONE))
        );

        switch (operation)
        {
            case OPERATION_MAKE_PORT :
                DEBUG_LOG("interrupt_region::make_port");
                return operation_make_port(owner, self);

            default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_lookup_result get_capability_by_descriptor(process &owner, a9n::word descriptor)
    {
        return owner.root_slot.component
            ->traverse_slot(descriptor, extract_depth(descriptor), a9n::BYTE_BITS);
    }

    capability_lookup_result get_empty_capability_slot_by_descriptor_and_index(
        process  &owner,
        a9n::word descriptor,
        a9n::word index
    )
    {
        return get_capability_by_descriptor(owner, descriptor)
            .and_then(
                [&](capability_slot *slot) -> capability_lookup_result
                {
                    if (slot->type != capability_type::NODE)
                    {
                        return capability_lookup_error::TERMINAL;
                    }

                    if (!slot->component)
                    {
                        // normally unreachable
                        return capability_lookup_error::UNEXPECTED;
                    }

                    return slot->component->retrieve_slot(index).and_then(
                        [&](capability_slot *target_slot) -> capability_lookup_result
                        {
                            if (target_slot->type != capability_type::NONE)
                            {
                                return capability_lookup_error::UNAVAILABLE;
                            }

                            return target_slot;
                        }
                    );
                }
            );
    }

    capability_result interrupt_region::operation_make_port(process &owner, capability_slot &self)
    {
        return with_message_registers<TARGET_NODE, TARGET_NODE_INDEX, IRQ_NUMBER>(
            owner,
            [&](a9n::word node_descriptor, a9n::word index, a9n::word irq) -> capability_result
            {
                return get_empty_capability_slot_by_descriptor_and_index(owner, node_descriptor, index)
                    .transform_error(convert_lookup_error)
                    .and_then(
                        [&](capability_slot *target_slot) -> capability_result
                        {
                            auto handler_result = irq_number_to_irq_notification_handler(irq);

                            if (!handler_result)
                            {
                                return capability_error::INVALID_ARGUMENT;
                            }

                            auto &handler = handler_result.unwrap();
                            if (handler->used)
                            {
                                return capability_error::ILLEGAL_OPERATION;
                            }
                            handler->used = true;

                            DEBUG_LOG("Make Interrupt Port for IRQ: %04llu\n", irq);

                            return try_configure_interrupt_port_slot(
                                       *target_slot,
                                       interrupt_port_core,
                                       handler->irq_number,
                                       0
                            )
                                .transform_error(convert_kernel_to_capability_error);
                        }
                    );
            }
        );
    }

    capability_result interrupt_region::revoke(capability_slot &self)
    {
        for (auto &handler : irq_notification_handlers)
        {
            DEBUG_LOG("Revoke IRQ Handler for IRQ: %04llu\n", handler.irq_number);
            handler.used = false;
            auto result  = handler.slot.try_remove_and_init();
            if (!result)
            {
                return result.transform_error(convert_kernel_to_capability_error);
            }
        }

        return {};
    }
}
