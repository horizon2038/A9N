#ifndef CAPABILITY_COMPONENT_HPP
#define CAPABILITY_COMPONENT_HPP

#include <kernel/capability/capability_local_state.hpp>
#include <kernel/ipc/message_buffer.hpp>
#include <library/common/types.hpp>
#include <library/capability/capability_descriptor.hpp>

#include <library/libc/string.hpp>

namespace kernel
{
    class capability_component;

    struct capability_slot
    {
        capability_component *component;
        capability_local_state *state;

        bool has_child()
        {
            return (
                state->family_node.depth
                > state->family_node.next_slot->state->family_node.depth
            );
        }

        bool is_same_slot(capability_slot *rhs)
        {
            auto is_same_data = std::memcmp(
                &state->data,
                &rhs->state->data,
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
        virtual ~capability_component() {};

        // called from user
        virtual common::error
            execute(capability_slot *this_slot, message_buffer *buffer)
            = 0;

        // called from node
        virtual common::error revoke() = 0;

        // for tree
        virtual capability_slot *retrieve_slot(common::word index) = 0;

        virtual capability_slot *traverse_slot(
            library::capability::capability_descriptor descriptor,
            common::word descriptor_max_bits,
            common::word descriptor_used_bits
        ) = 0;
    };
}

#endif
