#include "kernel/ipc/message_buffer.hpp"
#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_node.hpp>
#include <kernel/utility/logger.hpp>

#include <library/capability/capability_descriptor.hpp>
#include <library/capability/capability_node_operation.hpp>
#include <library/common/types.hpp>

namespace kernel
{
    capability_node::capability_node(
        common::word initial_ignore_bits,
        common::word initial_radix_bits,
        capability_component **initial_capability_slots
    )
        : capability_slots(initial_capability_slots)
        , ignore_bits(initial_ignore_bits)
        , radix_bits(initial_radix_bits)
    {
    }

    common::error capability_node::execute(message_buffer *buffer)
    {
        kernel::utility::logger::printk("execute : node\n");

        // 1. decode operation
        auto result = decode_operation(buffer);
        // 2. dispatch operation
        // 3.run operation

        return result;
    }

    common::error capability_node::decode_operation(message_buffer *buffer)
    {
        auto operation_type
            = static_cast<library::capability::node_operation_type>(
                buffer->get_element(2)
            );
        kernel::utility::logger::printk(
            "operation type : %llu\n",
            buffer->get_element(2)
        );

        switch (operation_type)
        {
            case library::capability::node_operation_type::COPY :
                {
                    kernel::utility::logger::printk("node : copy\n");
                    auto e = operation_copy(buffer);
                    return e;
                    break;
                }

            case library::capability::node_operation_type::MOVE :
                {
                    kernel::utility::logger::printk("node : move\n");
                    auto e = operation_move(buffer);
                    return e;
                    break;
                }

            case library::capability::node_operation_type::REVOKE :
                {
                    // the size of the message_buffer must be the same as one
                    // entry in the message buffer, so the size of the message
                    // buffer is the same as the size of a word.
                    kernel::utility::logger::printk("node : revoke\n");
                    // auto e = operation_revoke(buffer);
                    return 0;
                    break;
                }

            case library::capability::node_operation_type::REMOVE :
                {
                    kernel::utility::logger::printk("node : remove\n");
                    auto e = operation_remove(buffer);
                    return 0;
                }

            default :
                {
                    kernel::utility::logger::error("node : illegal operation!\n"
                    );
                    break;
                    // ERROR: illegal operation !
                }
        };
        return 0;
    }

    common::error capability_node::operation_copy(message_buffer *buffer)
    {
        auto destination_index = buffer->get_element(3);
        auto source_descriptor = buffer->get_element(4);
        auto source_depth = buffer->get_element(5);
        auto source_index = buffer->get_element(6);
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

    common::error capability_node::add_child(
        common::word index,
        capability_component *component
    )
    {
        if (component == nullptr)
        {
            // invalid component
            return -1;
        }

        if (!is_index_valid(index))
        {
            return -1;
        }

        if (capability_slots[index] != nullptr)
        {
            // slot is already used.
            // must delete first.
            return -1;
        }

        capability_slots[index] = component;

        return 0;
    }

    bool capability_node::is_index_valid(common::word index) const
    {
        auto index_max = static_cast<common::word>(1 << radix_bits);

        if (index >= index_max)
        {
            return false;
        }

        return true;
    }

    capability_component *capability_node::retrieve_child(common::word index)
    {
        if (!is_index_valid(index))
        {
            return nullptr;
        }

        auto component = capability_slots[index];

        if (component == nullptr)
        {
            return nullptr;
        }

        return component;
    }

    common::error capability_node::operation_move(message_buffer *buffer)
    {
        auto destination_index = buffer->get_element(3);
        auto source_descriptor = buffer->get_element(4);
        auto source_depth = buffer->get_element(5);
        auto source_index = buffer->get_element(6);
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

    common::error capability_node::operation_revoke(message_buffer *buffer)
    {
        auto target_index = buffer->get_element(3);
        auto e = revoke_child(target_index);
        return 0;
    }

    common::error capability_node::revoke_child(common::word index)
    {
        if (!is_index_valid(index))
        {
            return -1;
        }

        auto child_component = retrieve_child(index);
        child_component->revoke();
        return 0;
    }

    common::error capability_node::operation_remove(message_buffer *buffer)
    {
        auto target_index = buffer->get_element(3);
        remove_child(target_index);
        return 0;
    }

    common::error capability_node::remove_child(common::word index)
    {
        if (!is_index_valid(index))
        {
            return -1;
        }

        auto child_component = retrieve_child(index);
        child_component->remove();
        return 0;
    }

    common::error capability_node::revoke()
    {
        auto index_max = static_cast<common::word>(1 << radix_bits);
        for (auto i = 0; i < index_max; i++)
        {
            capability_slots[i] = nullptr;
        }
        return 0;
    }

    common::error capability_node::remove()
    {
        // node destructor
        return 0;
    }

    // recursively explores entries. this is a composite pattern that allows
    // handling single and multiple capabilities with the same interface.
    capability_component *capability_node::traverse(
        library::capability::capability_descriptor descriptor,
        common::word descriptor_max_bits, // usually WORD_BITS is used.
        common::word descriptor_used_bits
    )
    {
        auto entry = lookup_component(descriptor, descriptor_used_bits);
        if (entry == nullptr)
        {
            kernel::utility::logger::error("null entry !\n");
        }

        if (descriptor_used_bits == library::common::WORD_BITS)
        {
            return entry;
        }

        auto new_used_bits = calculate_used_bits(descriptor_used_bits);
        if (new_used_bits == descriptor_max_bits)
        {
            return entry;
        }
        return entry->traverse(descriptor, descriptor_max_bits, new_used_bits);
    }

    capability_component *capability_node::lookup_component(
        library::capability::capability_descriptor descriptor,
        common::word descriptor_used_bits
    )
    {
        kernel::utility::logger::printk("lookup_capability\n");
        auto index
            = calculate_capability_index(descriptor, descriptor_used_bits);
        auto component = index_to_capability_component(index);
        if (component == nullptr)
        {
            kernel::utility::logger::error("null entry !\n");
        }
        return component;
    }

    capability_component *
        capability_node::index_to_capability_component(common::word index)
    {
        auto index_max = 1 << radix_bits;
        if (index >= index_max)
        {
            kernel::utility::logger::error("index out of range !\n");
            return nullptr;
        }
        return capability_slots[index];
    }
}
