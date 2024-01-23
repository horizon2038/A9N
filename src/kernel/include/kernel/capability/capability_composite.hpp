#ifndef CAPABILITY_COMPOSITE_HPP
#define CAPABILITY_COMPOSITE_HPP

#include <kernel/ipc/message_buffer.hpp>
#include <library/common/types.hpp>
#include <library/capability/capability_descriptor.hpp>

namespace kernel
{
    class capability_composite
    {
        virtual ~capability_composite() {};

        virtual common::error execute(message_buffer *buffer) = 0;

        virtual common::error revoke() = 0;

        virtual capability_composite *traverse(
            library::capability::capability_descriptor descriptor,
            common::word descriptor_max_bits,
            common::word descriptor_used_bits
        );
    };
}

#endif
