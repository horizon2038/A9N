#include <kernel/capability/capability_node.hpp>

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_result.hpp>
#include <kernel/process/process.hpp>
#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

#include <hal/interface/process_manager.hpp>

namespace a9n::kernel
{
    capability_error convert_lookup_to_capability_error(capability_lookup_error e)
    {
        switch (e)
        {
            case capability_lookup_error::INDEX_OUT_OF_RANGE :
            case capability_lookup_error::TERMINAL :
                return capability_error::INVALID_ARGUMENT;
            default :
                return capability_error::INVALID_ARGUMENT;
        }
    }

    capability_node::capability_node(
        a9n::word        initial_ignore_bits,
        a9n::word        initial_radix_bits,
        capability_slot *initial_capability_slots
    )
        : capability_slots(initial_capability_slots)
        , ignore_bits(initial_ignore_bits)
        , radix_bits(initial_radix_bits)
    {
    }

    capability_result capability_node::execute(process &owner, capability_slot &self)
    {
        a9n::kernel::utility::logger::printk("execute : node\n");

        auto result = decode_operation(owner, self);

        return result;
    }

    capability_result capability_node::decode_operation(process &owner, capability_slot &self)
    {
        a9n::kernel::utility::logger::printk(
            "operation type : 0x%16llx\n",
            a9n::hal::get_message_register(owner, 1).unwrap_or(static_cast<a9n::word>(0))
        );

        auto target_operation = [&]() -> operation_type
        {
            return static_cast<operation_type>(
                a9n::hal::get_message_register(owner, OPERATION_TYPE).unwrap_or(static_cast<a9n::word>(0))
            );
        };

        switch (target_operation())
        {
            case COPY :
                {
                    a9n::kernel::utility::logger::printk("node : copy\n");
                    return operation_copy(owner, self);
                }

            case MOVE :
                {
                    a9n::kernel::utility::logger::printk("node : move\n");
                    return operation_move(owner, self);
                }

            case REVOKE :
                {
                    a9n::kernel::utility::logger::printk("node : revoke\n");
                    return operation_revoke(owner, self);
                }

            case MINT :
                {
                    a9n::kernel::utility::logger::printk("node : mint\n");
                    return operation_mint(owner, self);
                }

            case REMOVE :
                {
                    a9n::kernel::utility::logger::printk("node : remove\n");
                    return operation_remove(owner, self);
                }

            default :
                {
                    a9n::kernel::utility::logger::error("node : illegal "
                                                        "operation!");

                    return capability_error::ILLEGAL_OPERATION;
                }
        };

        return {};
    }

    capability_result capability_node::operation_copy(process &owner, capability_slot &self)
    {
        if (!(self.rights & capability_slot::object_rights::READ)
            && !(self.rights & capability_slot::object_rights::WRITE))
        {
            return capability_error::ILLEGAL_OPERATION;
        }

        auto get_message = [&](a9n::word index) -> a9n::word
        {
            return a9n::hal::get_message_register(owner, index).unwrap_or(static_cast<a9n::word>(0));
        };

        // it is not just about copying data.
        // after copying, we need to configure the Dependency Node.
        const auto destination_index = get_message(DESTINATION_INDEX);
        const auto source_descriptor = get_message(SOURCE_DESCRIPTOR);
        const auto source_index      = get_message(SOURCE_INDEX);

        return retrieve_slot(destination_index)
            .and_then(
                [=, this](capability_slot *slot) -> capability_lookup_result
                {
                    return slot->component->retrieve_slot(destination_index);
                }
            )
            .transform_error(convert_lookup_to_capability_error)
            .and_then(
                [&, this](capability_slot *destination_slot) -> capability_result
                {
                    return owner.root_slot.component
                        ->traverse_slot(source_descriptor, extract_depth(source_descriptor), a9n::BYTE_BITS)
                        .and_then(
                            [=, this](capability_slot *source_root_slot) -> capability_lookup_result
                            {
                                return source_root_slot->component->retrieve_slot(source_index);
                            }
                        )
                        .transform_error(convert_lookup_to_capability_error)
                        .and_then(
                            [=, this](capability_slot *source_slot) -> capability_result
                            {
                                if (!(source_slot->rights & capability_slot::object_rights::COPY)
                                    || (source_slot->type != capability_type::NONE))
                                {
                                    return capability_error::ILLEGAL_OPERATION;
                                }

                                destination_slot->type      = source_slot->type;
                                destination_slot->data      = source_slot->data;
                                destination_slot->component = source_slot->component;
                                destination_slot->rights    = source_slot->rights;
                                source_slot->insert_sibling(*destination_slot);

                                return {};
                            }
                        );
                }
            );
    }

    capability_result capability_node::operation_move(process &owner, capability_slot &self)
    {
        if (!(self.rights & capability_slot::object_rights::READ)
            && !(self.rights & capability_slot::object_rights::WRITE))
        {
            return capability_error::ILLEGAL_OPERATION;
        }

        auto get_message = [&](a9n::word index) -> a9n::word
        {
            return a9n::hal::get_message_register(owner, index).unwrap_or(static_cast<a9n::word>(0));
        };

        // it is not just about copying data.
        // after copying, we need to configure the Dependency Node.
        const auto destination_index = get_message(DESTINATION_INDEX);
        const auto source_descriptor = get_message(SOURCE_DESCRIPTOR);
        const auto source_index      = get_message(SOURCE_INDEX);

        return retrieve_slot(destination_index)
            .and_then(
                [=, this](capability_slot *slot) -> capability_lookup_result
                {
                    return slot->component->retrieve_slot(destination_index);
                }
            )
            .transform_error(convert_lookup_to_capability_error)
            .and_then(
                [&](capability_slot *destination_slot) -> capability_result
                {
                    return owner.root_slot.component
                        ->traverse_slot(source_descriptor, extract_depth(source_descriptor), a9n::BYTE_BITS)
                        .and_then(
                            [=, this](capability_slot *source_root_slot) -> capability_lookup_result
                            {
                                return source_root_slot->component->retrieve_slot(source_index);
                            }
                        )
                        .transform_error(convert_lookup_to_capability_error)
                        .and_then(
                            [=, this](capability_slot *source_slot) -> capability_result
                            {
                                if (source_slot->type != capability_type::NONE)
                                {
                                    a9n::kernel::utility::logger::error("slot is already used !\n");
                                    return capability_error::ILLEGAL_OPERATION;
                                }

                                // copy slot data
                                destination_slot->type         = source_slot->type;
                                destination_slot->data         = source_slot->data;
                                destination_slot->component    = source_slot->component;
                                destination_slot->rights       = source_slot->rights;

                                destination_slot->depth        = source_slot->depth;
                                destination_slot->preview_slot = source_slot->preview_slot;
                                destination_slot->next_slot    = source_slot->next_slot;

                                if (destination_slot->preview_slot)
                                {
                                    destination_slot->preview_slot->next_slot = destination_slot;
                                }

                                if (destination_slot->next_slot)
                                {
                                    destination_slot->next_slot->preview_slot = destination_slot;
                                }

                                source_slot->init();

                                return {};
                            }
                        );
                }
            );
    }

    capability_result capability_node::operation_mint(process &owner, capability_slot &self)
    {
        if (!(self.rights & capability_slot::object_rights::READ)
            && !(self.rights & capability_slot::object_rights::WRITE))
        {
            return capability_error::ILLEGAL_OPERATION;
        }

        return {};
    }

    capability_result capability_node::operation_revoke(process &owner, capability_slot &self)
    {
        return {};
    }

    capability_result capability_node::operation_remove(process &owner, capability_slot &self)
    {
        if (!(self.rights & capability_slot::object_rights::READ)
            && !(self.rights & capability_slot::object_rights::WRITE))
        {
            return capability_error::ILLEGAL_OPERATION;
        }

        /*
        auto destination_index  = buffer->get_message<0>();

        auto target_slot_result = self->component->retrieve_slot(destination_index);

        if (!target_slot_result)
        {
            return capability_error::INVALID_ARGUMENT;
        }

        // if the target capability is the last,
        // revoke the capability and then delete it.
        target_slot_result.unwrap()->component->revoke();

        auto target_depth  = target_slot_result.unwrap()->depth;
        auto next_depth    = (*target_slot_result)->next_slot->depth;

        auto preview_depth = (*target_slot_result)->next_slot->depth;

        auto target_slot   = (*target_slot_result);
        auto next_slot     = (*target_slot_result)->next_slot;
        auto preview_slot  = (*target_slot_result)->preview_slot;

        if (target_slot->is_same_slot(*next_slot) || target_slot->is_same_slot(*preview_slot))
        {
            target_slot->component->revoke();
        }

        // compare target depth
        // | A | A_a |
        // A : target_depth > next_depth = have child
        // A_a : target_depth < preview_depth = have child
        // 削除要件 :  dependency nodeにIdentityが等しいnodeが登録されているか?
        // is_same_slotを使えばできそう
        if (target_depth > next_depth || target_depth < preview_depth)
        {
            (*target_slot_result)->component->revoke();
        }

        // remove
        target_slot_result.unwrap()->data.fill(0);
        */

        return capability_error::DEBUG_UNIMPLEMENTED;
    }

    capability_lookup_result capability_node::retrieve_slot(a9n::word index)
    {
        [[unlikely]] if (!capability_slots)
        {
            return capability_lookup_error::UNAVAILABLE;
        }

        auto index_max = 1 << radix_bits;
        if (index >= index_max)
        {
            a9n::kernel::utility::logger::debug("index out of range !\n");
            return capability_lookup_error::INDEX_OUT_OF_RANGE;
        }

        return &(capability_slots[index]);
    }

    // recursively explores entries. this is a composite pattern that allows
    // handling single and multiple capabilities with the same interface.
    capability_lookup_result capability_node::traverse_slot(
        a9n::capability_descriptor descriptor,
        a9n::word                  descriptor_max_bits, // usually WORD_BITS is used.
        a9n::word                  descriptor_used_bits
    )
    {
        DEBUG_LOG("0x%016llx <> 0x%016llx <> 0x%016llx", descriptor, descriptor_max_bits, descriptor_used_bits);
        return lookup_slot(descriptor, descriptor_used_bits)
            .and_then(
                [=, this](capability_slot *slot) -> capability_lookup_result
                {
                    /*
                    if (descriptor_used_bits == a9n::WORD_BITS)
                    {
                        return slot;
                    }
                    */

                    auto new_used_bits = calculate_used_bits(descriptor_used_bits);
                    DEBUG_LOG("used bits : 0x%016llx -> 0x%016llx", descriptor_used_bits, new_used_bits);

                    DEBUG_LOG("used : 0x%016llx  max : 0x%016llx", new_used_bits, descriptor_max_bits);
                    if (new_used_bits == descriptor_max_bits)
                    {
                        return slot;
                    }

                    // when continuing the traverse, it must be
                    // checked wether a component exists in the slot.
                    // the capability_lookup_result is only for the existence
                    // of the slot.
                    if (!slot->component)
                    {
                        return capability_lookup_error::EMPTY;
                    }

                    return slot->component
                        ->traverse_slot(descriptor, descriptor_max_bits, new_used_bits)
                        .or_else(
                            [slot](capability_lookup_error e) -> capability_lookup_result
                            {
                                // if the search result for a capability_slot is
                                // terminal (typecally leaf), the slot itself is
                                // returned
                                // (terminate search even if descriptor remains)
                                if (e == capability_lookup_error::TERMINAL)
                                {
                                    return static_cast<capability_slot *>(slot);
                                }

                                return e;
                            }
                        );
                }
            );
    }

    capability_result capability_node::revoke(capability_slot &self)
    {
        /*
        if (!(self.rights & capability_slot::object_rights::READ)
            && !(self.rights & capability_slot::object_rights::WRITE))
        {
            return capability_error::ILLEGAL_OPERATION;
        }

        // recursively revoke all the capabilities
        for (auto start_slot = self.next_slot; start_slot->depth < self.depth;
             start_slot      = start_slot->next_slot)
        {
            if (!start_slot->component)
            {
                return {};
            }

            auto result = start_slot->component->revoke();
            if (!result)
            {
                return result;
            }
        }
        */

        return {};
    }

    capability_lookup_result
        capability_node::lookup_slot(a9n::capability_descriptor descriptor, a9n::word descriptor_used_bits)
    {
        DEBUG_LOG("lookup_capability");
        DEBUG_LOG("radix : %8llu", this->radix_bits);
        DEBUG_LOG("descriptor : 0x%016llx", descriptor);
        DEBUG_LOG("used bits : %8llu", descriptor_used_bits);

        auto index = calculate_capability_index(descriptor, descriptor_used_bits);
        DEBUG_LOG("index : %8llu", index);

        return retrieve_slot(index);
    }

}
