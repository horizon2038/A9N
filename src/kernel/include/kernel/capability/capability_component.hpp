#ifndef CAPABILITY_COMPONENT_HPP
#define CAPABILITY_COMPONENT_HPP

#include <kernel/capability/capability_local_state.hpp>
#include <kernel/ipc/message_buffer.hpp>
#include <kernel/types.hpp>
#include <liba9n/capability/capability_descriptor.hpp>

#include <liba9n/libc/string.hpp>

namespace a9n::kernel
{
    class capability_component;

    struct capability_slot
    {
        capability_component *component;

        capability_slot_data data;
        dependency_node      family_node;

        bool has_child()
        {
            return (
                family_node.depth > family_node.next_slot->family_node.depth
            );
        }

        bool is_same_slot(capability_slot *rhs)
        {
            auto is_same_data = liba9n::std::memcmp(
                &data,
                &rhs->data,
                sizeof(capability_slot_data)
            );

            if (is_same_data != 0)
            {
                return false;
            }

            // for memory object (e.g. generic, frame, page_table ...)
            if (component != rhs->component)
            {
                return false;
            }

            return true;
        }
    };

    class capability_component
    {
      public:
        // virtual ~capability_component() {};

        // called from user
        virtual a9n::error execute(
            capability_slot *this_slot,
            capability_slot *root_slot,
            message_buffer  *buffer
        ) = 0;

        // called from node
        virtual a9n::error revoke() = 0;

        // for tree
        virtual capability_slot *retrieve_slot(a9n::word index) = 0;

        virtual capability_slot *traverse_slot(
            liba9n::capability::capability_descriptor descriptor,
            a9n::word                                 descriptor_max_bits,
            a9n::word                                 descriptor_used_bits
        ) = 0;
    };
}

#endif
