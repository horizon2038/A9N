#include <kernel/capability/capability_node.hpp>

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_result.hpp>
#include <kernel/process/process.hpp>
#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

#include <hal/interface/process_manager.hpp>

namespace a9n::kernel
{
    namespace
    {
        struct node_reclamation_scope;
        node_reclamation_scope *active_node_reclamations { nullptr };

        // Reclamation is serialized by the giant lock. Track only active
        // finalizers, on their existing kernel stacks; Node lookup neither
        // reads this chain nor masks flags out of its geometry fields.
        struct node_reclamation_scope
        {
            const capability_node  &node;
            node_reclamation_scope *previous;

            explicit node_reclamation_scope(const capability_node &target)
                : node(target)
                , previous(active_node_reclamations)
            {
                active_node_reclamations = this;
            }

            ~node_reclamation_scope()
            {
                active_node_reclamations = previous;
            }
        };
    }

    bool capability_node::is_reclaiming(void) const
    {
        for (auto *scope = active_node_reclamations; scope; scope = scope->previous)
        {
            if (&scope->node == this)
            {
                return true;
            }
        }
        return false;
    }

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
        : ignore_bits(initial_ignore_bits)
        , radix_bits(initial_radix_bits)
        , capability_slots(initial_capability_slots)
    {
        // Node geometry is immutable. Validate once at construction rather
        // than repeating these checks during every capability invocation.
        if (!capability_slots || !is_geometry_valid(initial_ignore_bits, initial_radix_bits))
        {
            capability_slots = nullptr;
            return;
        }
        for (a9n::word i = 0; i < (static_cast<a9n::word>(1) << radix_bits); i++)
        {
            capability_slots[i].init();
        }
    }

    capability_result capability_node::execute(process &owner, capability_slot &self)
    {
        const auto operation = static_cast<operation_type>(
            a9n::hal::get_message_register(owner, OPERATION_TYPE).unwrap_or(a9n::word { 0 })
        );
        switch (operation)
        {
            case COPY :
            case MOVE :
            case MINT :
                return operation_transfer(owner, self, operation);
            case DEMOTE :
                return operation_demote(owner, self);
            case REVOKE :
                return operation_revoke(owner, self);
            case REMOVE :
                return operation_remove(owner, self);
            default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_result
        capability_node::operation_transfer(process &owner, capability_slot &self, operation_type operation)
    {
        if ((self.rights & (capability_slot::READ | capability_slot::WRITE))
            != (capability_slot::READ | capability_slot::WRITE))
        {
            return capability_error::ILLEGAL_OPERATION;
        }
        const auto destination_index
            = a9n::hal::get_message_register(owner, DESTINATION_INDEX).unwrap_or(a9n::word { 0 });
        const auto source_descriptor
            = a9n::hal::get_message_register(owner, SOURCE_DESCRIPTOR).unwrap_or(a9n::word { 0 });
        a9n::word new_rights = 0;
        if (operation == MINT)
        {
            new_rights = a9n::hal::get_message_register(owner, NEW_RIGHTS).unwrap_or(a9n::word { 0 });
        }

        auto destination_result = retrieve_slot(destination_index);
        if (!destination_result)
        {
            return convert_lookup_to_capability_error(destination_result.unwrap_error());
        }
        auto source_result = owner.root_slot.component->traverse_slot(
            source_descriptor,
            extract_depth(source_descriptor),
            a9n::BYTE_BITS
        );
        if (!source_result)
        {
            return convert_lookup_to_capability_error(source_result.unwrap_error());
        }
        auto *destination_slot = destination_result.unwrap();
        auto *source_slot      = source_result.unwrap();

        // MOVE transfers the existing Slot; COPY/MINT create a peer alias.
        if (operation == MOVE)
        {
            return try_move_capability_slot(*destination_slot, *source_slot)
                .transform_error(convert_kernel_to_capability_error);
        }
        if (!(source_slot->rights & capability_slot::COPY))
        {
            return capability_error::PERMISSION_DENIED;
        }
        // MINT may attenuate rights, never grant rights absent from the source.
        if (operation == MINT && (source_slot->rights & new_rights) != new_rights)
        {
            return capability_error::PERMISSION_DENIED;
        }

        auto copy_result = try_copy_capability_slot(*destination_slot, *source_slot)
                               .transform_error(convert_kernel_to_capability_error);
        if (!copy_result)
        {
            return copy_result;
        }
        if (operation == MINT)
        {
            destination_slot->rights = new_rights;
        }
        return {};
    }

    capability_result capability_node::operation_demote(process &owner, capability_slot &self)
    {
        if (!(self.rights & capability_slot::WRITE))
        {
            return capability_error::ILLEGAL_OPERATION;
        }
        const auto destination_index
            = a9n::hal::get_message_register(owner, DESTINATION_INDEX).unwrap_or(a9n::word { 0 });
        const auto new_rights
            = a9n::hal::get_message_register(owner, NEW_RIGHTS).unwrap_or(a9n::word { 0 });

        auto slot_result = retrieve_slot(destination_index);
        if (!slot_result)
        {
            return convert_lookup_to_capability_error(slot_result.unwrap_error());
        }
        auto *target_slot = slot_result.unwrap();
        if ((target_slot->rights & new_rights) != new_rights)
        {
            return capability_error::PERMISSION_DENIED;
        }
        target_slot->rights = new_rights;
        return {};
    }

    capability_result capability_node::operation_revoke(process &owner, capability_slot &self)
    {
        if ((self.rights & (capability_slot::READ | capability_slot::WRITE))
            != (capability_slot::READ | capability_slot::WRITE))
        {
            return capability_error::PERMISSION_DENIED;
        }
        const auto destination_index
            = a9n::hal::get_message_register(owner, DESTINATION_INDEX).unwrap_or(a9n::word { 0 });

        auto slot_result = retrieve_slot(destination_index);
        if (!slot_result)
        {
            return convert_lookup_to_capability_error(slot_result.unwrap_error());
        }
        auto *target_slot = slot_result.unwrap();
        if (target_slot->type == capability_type::NONE || !target_slot->component)
        {
            return capability_error::INVALID_DESCRIPTOR;
        }

        // Removing a weak alias is allowed; resetting its shared object is not.
        if ((target_slot->rights & (capability_slot::READ | capability_slot::WRITE))
            != (capability_slot::READ | capability_slot::WRITE))
        {
            return capability_error::PERMISSION_DENIED;
        }
        return revoke_slot(*target_slot).transform_error(convert_kernel_to_capability_error);
    }

    capability_result capability_node::operation_remove(process &owner, capability_slot &self)
    {
        if ((self.rights & (capability_slot::READ | capability_slot::WRITE))
            != (capability_slot::READ | capability_slot::WRITE))
        {
            return capability_error::PERMISSION_DENIED;
        }
        const auto destination_index
            = a9n::hal::get_message_register(owner, DESTINATION_INDEX).unwrap_or(a9n::word { 0 });

        auto slot_result = retrieve_slot(destination_index);
        if (!slot_result)
        {
            return convert_lookup_to_capability_error(slot_result.unwrap_error());
        }
        return remove_slot(*slot_result.unwrap()).transform_error(convert_kernel_to_capability_error);
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
                [this, descriptor, descriptor_max_bits, descriptor_used_bits](capability_slot *slot)
                    -> capability_lookup_result
                {
                    auto new_used_bits = calculate_used_bits(descriptor_used_bits);
                    DEBUG_LOG("used bits : 0x%016llx -> 0x%016llx", descriptor_used_bits, new_used_bits);

                    DEBUG_LOG("used : 0x%016llx  max : 0x%016llx", new_used_bits, descriptor_max_bits);
                    if (new_used_bits == descriptor_max_bits)
                    {
                        return slot;
                    }

                    // Lookup establishes Slot existence, not component existence.
                    if (!slot->component) [[unlikely]]
                    {
                        return capability_lookup_error::EMPTY;
                    }

                    return slot->component->traverse_slot(descriptor, descriptor_max_bits, new_used_bits);
                }
            );
    }

    capability_result capability_node::revoke(capability_slot &self)
    {
        // This callback is reached only through an authorized Node operation
        // or ancestor reclamation. Object-use rights do not authorize cleanup.
        if (is_reclaiming())
        {
            return {};
        }
        node_reclamation_scope scope(*this);
        for (a9n::word i = 0; i < (static_cast<a9n::word>(1) << radix_bits); i++)
        {
            if (!capability_slots) [[unlikely]]
            {
                return capability_error::ILLEGAL_OPERATION;
            }

            auto &slot = capability_slots[i];
            // Keep the invocation slot pinned if the Node contains itself.
            // Other self aliases can be detached without recursive destruction.
            if (&slot == &self)
            {
                continue;
            }
            if (slot.is_same_object(self))
            {
                slot.remove_from_dependency_node();
                slot.init();
                continue;
            }
            if (slot.type != capability_type::NONE)
            {
                auto result = remove_slot(slot);
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
