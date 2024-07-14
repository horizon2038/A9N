#include "kernel/ipc/ipc_buffer.hpp"
#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_node.hpp>
#include <kernel/utility/logger.hpp>

#include <liba9n/capability/capability_descriptor.hpp>
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

    a9n::error capability_node::execute(
        capability_slot *this_slot,
        capability_slot *root_slot,
        ipc_buffer      *buffer
    )
    {
        a9n::kernel::utility::logger::printk("execute : node\n");

        // 1. decode operation
        auto result = decode_operation(buffer);
        // 2. dispatch operation
        // 3.run operation

        return result;
    }

    a9n::error capability_node::decode_operation(ipc_buffer *buffer)
    {
        auto operation_type
            = static_cast<liba9n::capability::node_operation_type>(
                buffer->message_tag
            );
        a9n::kernel::utility::logger::printk(
            "operation type : %llu\n",
            buffer->message_tag
        );

        switch (operation_type)
        {
            case liba9n::capability::node_operation_type::COPY :
                {
                    a9n::kernel::utility::logger::printk("node : copy\n");
                    auto e = operation_copy(buffer);
                    return e;
                    break;
                }

            case liba9n::capability::node_operation_type::MOVE :
                {
                    a9n::kernel::utility::logger::printk("node : move\n");
                    auto e = operation_move(buffer);
                    return e;
                    break;
                }

            case liba9n::capability::node_operation_type::REVOKE :
                {
                    // the size of the ipc_buffer must be the same as one
                    // entry in the message buffer, so the size of the message
                    // buffer is the same as the size of a word.
                    a9n::kernel::utility::logger::printk("node : revoke\n");
                    // auto e = operation_revoke(buffer);
                    return 0;
                    break;
                }

            case liba9n::capability::node_operation_type::REMOVE :
                {
                    a9n::kernel::utility::logger::printk("node : remove\n");
                    return 0;
                }

            default :
                {
                    a9n::kernel::utility::logger::error(
                        "node : illegal operation!\n"
                    );
                    break;
                    // ERROR: illegal operation !
                }
        };
        return 0;
    }

    a9n::error capability_node::operation_copy(ipc_buffer *buffer)
    {
        auto destination_index = buffer->get_message<0>();
        auto source_descriptor = buffer->get_message<1>();
        auto source_depth      = buffer->get_message<2>();
        auto source_index      = buffer->get_message<3>();
        // TODO: create root_node
        // auto source_root_node = root_node->traverse(source_descriptor,
        // source_depth, 0);
        // auto source_component =
        // source_root_node->retrieve_child(source_index);
        // auto e =
        // add_child(destination_index, source_component);
        //
        // return e;
        return 0;
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

    a9n::error capability_node::operation_move(ipc_buffer *buffer)
    {
        auto destination_index = buffer->get_message<0>();
        auto source_descriptor = buffer->get_message<1>();
        auto source_depth      = buffer->get_message<2>();
        auto source_index      = buffer->get_message<3>();
        // TODO: create root_node
        // auto source_root_node = root_node->traverse(source_descriptor,
        // source_depth, 0);
        // auto source_component =
        // source_root_node->retrieve_child(source_index);
        // auto e =
        // add_child(destination_index, source_component);
        // e = source_root_node->remove_child(source_index);
        //
        // return e;
        return 0;
    }

    // USECASE of Generic::Convert
    // convert -> create new slot(component and local_state)
    // target_node.add_slot(index, new_slot)
    //

    // recursively explores entries. this is a composite pattern that allows
    // handling single and multiple capabilities with the same interface.
    capability_slot *capability_node::traverse_slot(
        liba9n::capability::capability_descriptor descriptor,
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
        if (next_slot == nullptr)
        {
            return slot;
        }

        return next_slot;
    }

    capability_slot *capability_node::lookup_slot(
        liba9n::capability::capability_descriptor descriptor,
        a9n::word                                 descriptor_used_bits
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
