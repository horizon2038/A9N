#ifndef CAPABILITY_COMPONENT_HPP
#define CAPABILITY_COMPONENT_HPP

#include <kernel/capability/capability_local_state.hpp>
#include <kernel/ipc/message_buffer.hpp>
#include <library/common/types.hpp>
#include <library/capability/capability_descriptor.hpp>

namespace kernel
{
    class capability_component;

    struct capability_slot
    {
        capability_component *component;
        capability_local_state *state;
    };

    class capability_component
    {
      public:
        virtual ~capability_component() {};

        // called from user
        virtual common::error execute(message_buffer *buffer) = 0;

        // called from each capabilities
        virtual common::error
            add_child(common::word index, capability_component *component)
            = 0;

        virtual capability_slot *retrieve_slot(common::word index) = 0;

        virtual common::error revoke_child(common::word index) = 0;

        virtual common::error remove_child(common::word index) = 0;

        // called from node
        virtual common::error revoke() = 0;

        virtual common::error remove() = 0;

        virtual capability_slot *traverse_slot(
            library::capability::capability_descriptor descriptor,
            common::word descriptor_max_bits,
            common::word descriptor_used_bits
        ) = 0;
    };
}

#endif
