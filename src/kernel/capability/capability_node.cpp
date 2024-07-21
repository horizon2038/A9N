#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_node.hpp>
#include <kernel/utility/logger.hpp>

#include <liba9n/capability/capability_node_operation.hpp>
#include <kernel/types.hpp>

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

    capability_error capability_node::execute(
        capability_slot *this_slot,
        capability_slot *root_slot,
        ipc_buffer      *buffer
    )
    {
        a9n::kernel::utility::logger::printk("execute : node\n");

        auto result = decode_operation(buffer);

        return result;
    }

    capability_error capability_node::decode_operation(ipc_buffer *buffer)
    {
        a9n::kernel::utility::logger::printk(
            "operation type : %llu\n",
            buffer->message_tag
        );

        switch (auto operation_type
                = static_cast<liba9n::capability::node_operation_type>(
                    buffer->message_tag
                ))
        {
            case liba9n::capability::node_operation_type::COPY :
                {
                    a9n::kernel::utility::logger::printk("node : copy\n");
                    return operation_copy(buffer);
                }

            case liba9n::capability::node_operation_type::MOVE :
                {
                    a9n::kernel::utility::logger::printk("node : move\n");
                    return operation_move(buffer);
                }

            case liba9n::capability::node_operation_type::REVOKE :
                {
                    a9n::kernel::utility::logger::printk("node : revoke\n");
                    return operation_revoke(buffer);
                }

            case liba9n::capability::node_operation_type::REMOVE :
                {
                    a9n::kernel::utility::logger::printk("node : remove\n");
                    return operation_remove(buffer);
                }

            default :
                {
                    a9n::kernel::utility::logger::error(
                        "node : illegal operation!\n"
                    );

                    return capability_error_type::ILLEGAL_OPERATION;
                }
        };

        return {};
    }

    capability_error capability_node::operation_copy(ipc_buffer *buffer)
    {
        auto destination_index = buffer->get_message<0>();
        auto source_descriptor = buffer->get_message<1>();
        auto source_depth      = buffer->get_message<2>();
        auto source_index      = buffer->get_message<3>();

        return capability_error_type::DEBUG_UNIMPLEMENTED;
    }

    // return of empty slots is allowed
    capability_slot *capability_node::retrieve_slot(a9n::word index)
    {
        auto index_max = 1 << radix_bits;
        if (index >= index_max)
        {
            a9n::kernel::utility::logger::debug("index out of range !\n");
            return nullptr;
        }
        return &(capability_slots[index]);
    }

    capability_error capability_node::operation_move(ipc_buffer *buffer)
    {
        auto destination_index = buffer->get_message<0>();
        auto source_descriptor = buffer->get_message<1>();
        auto source_depth      = buffer->get_message<2>();
        auto source_index      = buffer->get_message<3>();

        return capability_error_type::DEBUG_UNIMPLEMENTED;
    }

    capability_error capability_node::operation_revoke(ipc_buffer *buffer)
    {
        return capability_error_type::DEBUG_UNIMPLEMENTED;
    }

    capability_error capability_node::operation_remove(ipc_buffer *buffer)
    {
        return capability_error_type::DEBUG_UNIMPLEMENTED;
    }

    // recursively explores entries. this is a composite pattern that allows
    // handling single and multiple capabilities with the same interface.
    capability_slot *capability_node::traverse_slot(
        a9n::capability_descriptor descriptor,
        a9n::word descriptor_max_bits, // usually WORD_BITS is used.
        a9n::word descriptor_used_bits
    )
    {
        auto slot = lookup_slot(descriptor, descriptor_used_bits);
        if (slot == nullptr)
        {
            a9n::kernel::utility::logger::error("null entry !\n");
        }

        if (descriptor_used_bits == a9n::WORD_BITS)
        {
            return slot;
        }

        auto new_used_bits = calculate_used_bits(descriptor_used_bits);
        if (new_used_bits == descriptor_max_bits)
        {
            return slot;
        }

        auto next_slot = slot->component->traverse_slot(
            descriptor,
            descriptor_max_bits,
            new_used_bits
        );

        // if next_slot has no children (i.e., it is a terminal)
        // HACK: replace to result<capability_slot*, capability_traverse_error>
        // - to distinguish between missing slots and terminations
        if (next_slot == nullptr)
        {
            return slot;
        }

        return next_slot;
    }

    capability_slot *capability_node::lookup_slot(
        a9n::capability_descriptor descriptor,
        a9n::word                  descriptor_used_bits
    )
    {
        a9n::kernel::utility::logger::printk("lookup_capability\n");
        auto index
            = calculate_capability_index(descriptor, descriptor_used_bits);
        auto slot = retrieve_slot(index);
        if (slot == nullptr)
        {
            a9n::kernel::utility::logger::error("null entry !\n");
        }

        return slot;
    }
}
