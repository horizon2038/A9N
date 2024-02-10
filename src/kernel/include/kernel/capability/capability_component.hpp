#ifndef CAPABILITY_COMPONENT_HPP
#define CAPABILITY_COMPONENT_HPP

#include <kernel/ipc/message_buffer.hpp>
#include <library/common/types.hpp>
#include <library/capability/capability_descriptor.hpp>

namespace kernel
{
    class capability_component
    {
      public:
        virtual ~capability_component() {};

        virtual common::error execute(message_buffer *buffer) = 0;

        virtual common::error update() = 0;

        virtual common::error revoke() = 0;
        virtual common::error revoke_all() = 0;

        virtual capability_component *traverse(
            library::capability::capability_descriptor descriptor,
            common::word descriptor_max_bits,
            common::word descriptor_used_bits
        );
    };
}

#endif
