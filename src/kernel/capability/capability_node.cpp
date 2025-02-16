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
        DEBUG_LOG("capability_lookup_error : %s\n", capability_lookup_error_to_string(e));
        switch (e)
        {
            case capability_lookup_error::INDEX_OUT_OF_RANGE :
                [[fallthrough]];
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
        auto result = decode_operation(owner, self);

        return result;
    }

    capability_result capability_node::decode_operation(process &owner, capability_slot &self)
    {
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
                    DEBUG_LOG("node::copy");
                    return operation_copy(owner, self);
                }

            case MOVE :
                {
                    DEBUG_LOG("node::move");
                    return operation_move(owner, self);
                }

            case MINT :
                {
                    DEBUG_LOG("node::mint");
                    return operation_mint(owner, self);
                }

            case DEMOTE :
                {
                    DEBUG_LOG("node::demote");
                    return operation_demote(owner, self);
                }

            case REVOKE :
                {
                    DEBUG_LOG("node::revoke");
                    return operation_revoke(owner, self);
                }

            case REMOVE :
                {
                    DEBUG_LOG("node::remove");
                    return operation_remove(owner, self);
                }

            default :
                {
                    DEBUG_LOG("node : illegal operation");
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

        return retrieve_slot(destination_index)
            .transform_error(convert_lookup_to_capability_error)
            .and_then(
                [&, this](capability_slot *destination_slot) -> capability_result
                {
                    return owner.root_slot.component
                        ->traverse_slot(source_descriptor, extract_depth(source_descriptor), a9n::BYTE_BITS)
                        .transform_error(convert_lookup_to_capability_error)
                        .and_then(
                            [=, this](capability_slot *source_slot) -> capability_result
                            {
                                if (!(source_slot->rights & capability_slot::object_rights::COPY))
                                {
                                    DEBUG_LOG("source slot does not have COPY rights");
                                    return capability_error::PERMISSION_DENIED;
                                }

                                DEBUG_LOG("copy 0x%016llx -> 0x%016llx", source_slot, destination_slot);
                                return try_copy_capability_slot(*destination_slot, *source_slot)
                                    .transform_error(convert_kernel_to_capability_error);
                            }
                        );
                }
            );
    }

    capability_result capability_node::operation_move(process &owner, capability_slot &self)
    {
        if (!(self.rights & capability_slot::object_rights::READ)
            || !(self.rights & capability_slot::object_rights::WRITE))
        {
            return capability_error::ILLEGAL_OPERATION;
        }

        auto get_message = [&](a9n::word index) -> a9n::word
        {
            return a9n::hal::get_message_register(owner, index).unwrap_or(static_cast<a9n::word>(0));
        };

        const auto destination_index = get_message(DESTINATION_INDEX);
        const auto source_descriptor = get_message(SOURCE_DESCRIPTOR);

        return retrieve_slot(destination_index)
            .transform_error(convert_lookup_to_capability_error)
            .and_then(
                [&](capability_slot *destination_slot) -> capability_result
                {
                    return owner.root_slot.component
                        ->traverse_slot(source_descriptor, extract_depth(source_descriptor), a9n::BYTE_BITS)
                        .transform_error(convert_lookup_to_capability_error)
                        .and_then(
                            [=, this](capability_slot *source_slot) -> capability_result
                            {
                                // it is not just about copying data.
                                // after copying, we need to configure the Dependency Node.
                                // try_move_capability_slot : re-configure the Dependency Node
                                DEBUG_LOG("move 0x%016llx -> 0x%016llx", source_slot, destination_slot);
                                return try_move_capability_slot(*destination_slot, *source_slot)
                                    .transform_error(convert_kernel_to_capability_error);
                            }
                        );
                }
            );
    }

    capability_result capability_node::operation_mint(process &owner, capability_slot &self)
    {
        if ((!(self.rights & capability_slot::object_rights::READ)
             || !(self.rights & capability_slot::object_rights::WRITE)))
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
        const auto new_rights        = get_message(NEW_RIGHTS);

        return retrieve_slot(destination_index)
            .transform_error(convert_lookup_to_capability_error)
            .and_then(
                [&, this](capability_slot *destination_slot) -> capability_result
                {
                    return owner.root_slot.component
                        ->traverse_slot(source_descriptor, extract_depth(source_descriptor), a9n::BYTE_BITS)
                        .transform_error(convert_lookup_to_capability_error)
                        .and_then(
                            [=, this](capability_slot *source_slot) -> capability_result
                            {
                                if (!(source_slot->rights & capability_slot::object_rights::COPY))
                                {
                                    DEBUG_LOG("source slot does not have COPY rights");
                                    return capability_error::PERMISSION_DENIED;
                                }

                                // new_rights must be a subset of source_slot->rights
                                if ((source_slot->rights & new_rights) != new_rights)
                                {
                                    DEBUG_LOG("new rights is not subset of source rights");
                                    return capability_error::PERMISSION_DENIED;
                                }

                                return try_copy_capability_slot(*destination_slot, *source_slot)
                                    .transform_error(convert_kernel_to_capability_error)
                                    .and_then(
                                        [&](void) -> capability_result
                                        {
                                            destination_slot->rights = new_rights;
                                            return {};
                                        }
                                    );
                            }
                        );
                }
            );

        return {};
    }

    capability_result capability_node::operation_demote(process &owner, capability_slot &self)
    {
        if (!(self.rights & capability_slot::object_rights::WRITE))
        {
            return capability_error::ILLEGAL_OPERATION;
        }

        auto get_message = [&](a9n::word index) -> a9n::word
        {
            return a9n::hal::get_message_register(owner, index).unwrap_or(static_cast<a9n::word>(0));
        };

        const auto destination_index = get_message(DESTINATION_INDEX);
        const auto new_rights        = get_message(NEW_RIGHTS);

        return retrieve_slot(destination_index)
            .transform_error(convert_lookup_to_capability_error)
            .and_then(
                [&](capability_slot *slot) -> capability_result
                {
                    if (!(slot->rights & capability_slot::object_rights::READ)
                        || !(slot->rights & capability_slot::object_rights::WRITE))
                    {
                        DEBUG_LOG("no READ or WRITE rights");
                        return capability_error::ILLEGAL_OPERATION;
                    }

                    // check if the new_rights is a subset of the source_slot->rights
                    if ((slot->rights & new_rights) != new_rights)
                    {
                        DEBUG_LOG("new rights is not subset of source rights");
                        return capability_error::PERMISSION_DENIED;
                    }

                    slot->rights = new_rights;

                    return {};
                }
            );

        return {};
    }

    capability_result capability_node::operation_revoke(process &owner, capability_slot &self)
    {
        auto get_message = [&](a9n::word index) -> a9n::word
        {
            return a9n::hal::get_message_register(owner, index).unwrap_or(static_cast<a9n::word>(0));
        };

        const auto destination_index = get_message(DESTINATION_INDEX);

        return retrieve_slot(destination_index)
            .transform_error(convert_lookup_to_capability_error)
            .and_then(
                [&](capability_slot *slot) -> capability_result
                {
                    if (!(slot->rights & capability_slot::object_rights::READ)
                        || !(slot->rights & capability_slot::object_rights::WRITE))
                    {
                        return capability_error::ILLEGAL_OPERATION;
                    }

                    if (slot->component == self.component)
                    {
                        // NOTE: storing node recursively is permitted.
                        // however, such a specification causes a problem of infinite revoke loop.
                        // therefore, it is necessary to check if the target is the same as itself
                        // (same component/capability-instance)
                        return revoke(self);
                    }

                    return slot->component->revoke(*slot);
                }
            );

        return {};
    }

    capability_result capability_node::operation_remove(process &owner, capability_slot &self)
    {
        if (!(self.rights & capability_slot::object_rights::READ)
            && !(self.rights & capability_slot::object_rights::WRITE))
        {
            DEBUG_LOG("no READ or WRITE rights");
            return capability_error::PERMISSION_DENIED;
        }

        auto get_message = [&](a9n::word index) -> a9n::word
        {
            return a9n::hal::get_message_register(owner, index).unwrap_or(static_cast<a9n::word>(0));
        };

        const auto destination_index = get_message(DESTINATION_INDEX);

        return retrieve_slot(destination_index)
            .transform_error(convert_lookup_to_capability_error)
            .and_then(
                [&](capability_slot *slot) -> capability_result
                {
                    // revoke!
                    if (!(slot->rights & capability_slot::object_rights::READ)
                        || !(slot->rights & capability_slot::object_rights::WRITE))
                    {
                        DEBUG_LOG("no READ or WRITE rights");
                        return capability_error::PERMISSION_DENIED;
                    }

                    auto result = slot->try_remove_and_init();
                    if (!result)
                    {
                        return result.transform_error(convert_kernel_to_capability_error);
                    }

                    DEBUG_LOG("remove : successfully finished");
                    return {};
                }
            );

        return capability_error::DEBUG_UNIMPLEMENTED;
    }

    capability_lookup_result capability_node::retrieve_slot(a9n::word index)
    {
        if (!capability_slots) [[unlikely]]
        {
            return capability_lookup_error::UNAVAILABLE;
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
                [this, descriptor, descriptor_max_bits, descriptor_used_bits](capability_slot *slot
                ) -> capability_lookup_result
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
        if (!(self.rights & capability_slot::object_rights::READ)
            && !(self.rights & capability_slot::object_rights::WRITE))
        {
            return capability_error::ILLEGAL_OPERATION;
        }

        // remove
        for (auto i = 0; i < (static_cast<a9n::word>(1) << radix_bits); i++)
        {
            if (!capability_slots) [[unlikely]]
            {
                DEBUG_LOG("capability_slots is null");
                return capability_error::ILLEGAL_OPERATION;
            }

            auto &slot = capability_slots[i];
            if (slot.is_same_slot(self))
            {
                auto result = slot.try_remove_and_init();
                if (!result)
                {
                    return capability_error::ILLEGAL_OPERATION;
                }
            }
        }

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
