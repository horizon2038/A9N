#include "kernel/capability/capability_result.hpp"
#include <kernel/capability/capability_node.hpp>

#include <kernel/capability/capability_component.hpp>
#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

#include <kernel/capability/operation/capability_node_operation.hpp>

namespace a9n::kernel
{
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

    capability_result capability_node::execute(
        ipc_buffer      *buffer,
        capability_slot *this_slot,
        capability_slot *root_slot
    )
    {
        a9n::kernel::utility::logger::printk("execute : node\n");

        auto result = decode_operation(buffer, this_slot, root_slot);

        return result;
    }

    capability_result capability_node::decode_operation(
        ipc_buffer      *buffer,
        capability_slot *this_slot,
        capability_slot *root_slot
    )
    {
        a9n::kernel::utility::logger::printk(
            "operation type : %llu\n",
            buffer->message_tag
        );

        switch (auto operation_type
                = static_cast<a9n::kernel::capability_node_operation>(buffer->message_tag
                ))
        {
            case capability_node_operation::COPY :
                {
                    a9n::kernel::utility::logger::printk("node : copy\n");
                    return operation_copy(buffer, this_slot, root_slot);
                }

            case capability_node_operation::MOVE :
                {
                    a9n::kernel::utility::logger::printk("node : move\n");
                    return operation_move(buffer, this_slot, root_slot);
                }

            case capability_node_operation::REVOKE :
                {
                    a9n::kernel::utility::logger::printk("node : revoke\n");
                    return operation_revoke(buffer, this_slot, root_slot);
                }

            case capability_node_operation::REMOVE :
                {
                    a9n::kernel::utility::logger::printk("node : remove\n");
                    return operation_remove(buffer, this_slot, root_slot);
                }

            default :
                {
                    a9n::kernel::utility::logger::error("node : illegal "
                                                        "operation!\n");

                    return capability_error::ILLEGAL_OPERATION;
                }
        };

        return {};
    }

    capability_result capability_node::operation_copy(
        ipc_buffer      *buffer,
        capability_slot *this_slot,
        capability_slot *root_slot
    )
    {
        if (!root_slot)
        {
            return capability_error::INVALID_ARGUMENT;
        }

        // it is not just about copying data.
        // after copying, we need to configure the Dependency Node.
        namespace argument = capability_node_copy_argument;

        auto destination_index = buffer->get_message<argument::DESTINATION_INDEX>();
        auto source_descriptor = buffer->get_message<argument::SOURCE_DESCRIPTOR>();
        auto source_depth = buffer->get_message<argument::SOURCE_DEPTH>();
        auto source_index = buffer->get_message<argument::SOURCE_INDEX>();

        auto destination_slot_result = retrieve_slot(destination_index);
        if (!destination_slot_result)
        {
            return capability_error::INVALID_DESCRIPTOR;
        }

        auto source_slot_result
            = root_slot->component
                  ->traverse_slot(source_descriptor, source_depth, 0)
                  .and_then(
                      [=, this](capability_slot *slot) -> capability_lookup_result
                      {
                          return slot->component->retrieve_slot(source_index);
                      }
                  );
        if (source_slot_result)
        {
            return capability_error::INVALID_DESCRIPTOR;
        }

        source_slot_result.unwrap()->component
            = destination_slot_result.unwrap()->component;
        destination_slot_result.unwrap()->data = source_slot_result.unwrap()->data;

        source_slot_result.unwrap()->family_node.next_slot
            = destination_slot_result.unwrap();
        destination_slot_result.unwrap()->family_node.preview_slot
            = source_slot_result.unwrap();

        return capability_error::DEBUG_UNIMPLEMENTED;
    }

    capability_result capability_node::operation_move(
        ipc_buffer      *buffer,
        capability_slot *this_slot,
        capability_slot *root_slot
    )
    {
        namespace argument = capability_node_move_argument;

        auto destination_index = buffer->get_message<argument::DESTINATION_INDEX>();
        auto source_descriptor = buffer->get_message<argument::SOURCE_DESCRIPTOR>();
        auto source_depth = buffer->get_message<argument::SOURCE_DEPTH>();
        auto source_index = buffer->get_message<argument::SOURCE_INDEX>();

        return capability_error::DEBUG_UNIMPLEMENTED;
    }

    capability_result capability_node::operation_revoke(
        ipc_buffer      *buffer,
        capability_slot *this_slot,
        capability_slot *root_slot
    )
    {
        auto this_depth = this_slot->family_node.depth;

        for (auto start_slot = this_slot->family_node.next_slot;
             start_slot->family_node.depth < this_depth;
             start_slot = start_slot->family_node.next_slot)
        {
            if (!start_slot->component)
            {
                return capability_error::ILLEGAL_OPERATION;
            }

            start_slot->component->revoke();
        }

        return {};
    }

    capability_result capability_node::operation_remove(
        ipc_buffer      *buffer,
        capability_slot *this_slot,
        capability_slot *root_slot
    )
    {
        auto destination_index = buffer->get_message<0>();

        auto target_slot_result
            = this_slot->component->retrieve_slot(destination_index);

        if (!target_slot_result)
        {
            return capability_error::INVALID_ARGUMENT;
        }

        // if the target capability is the last,
        // revoke the capability and then delete it.
        target_slot_result.unwrap()->component->revoke();

        auto target_depth = target_slot_result.unwrap()->family_node.depth;
        auto next_depth
            = (*target_slot_result)->family_node.next_slot->family_node.depth;

        auto preview_depth
            = (*target_slot_result)->family_node.next_slot->family_node.depth;

        auto target_slot  = (*target_slot_result);
        auto next_slot    = (*target_slot_result)->family_node.next_slot;
        auto preview_slot = (*target_slot_result)->family_node.preview_slot;

        if (target_slot->is_same_slot(next_slot)
            || target_slot->is_same_slot(preview_slot))
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
        a9n::word descriptor_max_bits, // usually WORD_BITS is used.
        a9n::word descriptor_used_bits
    )
    {
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

    capability_lookup_result capability_node::lookup_slot(
        a9n::capability_descriptor descriptor,
        a9n::word                  descriptor_used_bits
    )
    {
        a9n::kernel::utility::logger::printk("lookup_capability\n");

        auto index = calculate_capability_index(descriptor, descriptor_used_bits);
        auto slot = retrieve_slot(index);
        if (!slot)
        {
            // a9n::kernel::utility::logger::error("null entry !\n");
            return capability_lookup_error::UNAVAILABLE;
        }

        return slot;
    }

}
