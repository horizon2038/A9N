#ifndef CAPABILITY_HPP
#define CAPABILITY_HPP

#include <kernel/ipc/message_buffer.hpp>
#include <library/capability/capability_descriptor.hpp>
#include <library/common/types.hpp>

namespace kernel
{
    enum class capability_type : uint8_t
    {
        // NULL OBJECT PATTERN
        EMPTY = 0x00,

        // NODE type represents a node that can store capabilities itself.
        // They are used for exploration and are casted to node.
        NODE = 0x01,

        // DEFAULT capability_type
        OBJECT = 0x02,

        /*
        // REAL type represents objects that have a separate instance created
        // in memory, such as process_control_block.
        REAL = 0x02,

        // VIRTUAL type represents objects that do not have an instance created
        // in memory, such as frame or generic.
        // it can also be used for debugging capabilities.
        VIRTUAL = 0x03,
        */
    };

    // capability_entry_state is an essential presence for capability.execute().
    // This enables providing a common interface for capabilities such as
    // generic, frame, and others where we do not want to have a physical entity
    // in memory for each. This enhances extensibility and maintainability.

    struct capability_entry_state;
    struct capability_entry;

    class capability
    {
      public:
        virtual common::error
            execute(message_buffer *buffer, capability_entry *stored_entry)
            = 0;

        virtual common::error revoke() = 0;

        virtual capability_type type() = 0;

        virtual capability_entry *traverse_entry(
            library::capability::capability_descriptor descriptor,
            common::word descriptor_max_bits,
            common::word descriptor_used_bits
        ) = 0;
    };

    struct capability_entry
    {
        capability *capability_pointer;
        capability_entry_state state;

        common::error execute(message_buffer *buffer)
        {
            return capability_pointer->execute(buffer, this);
        }

        common::error revoke()
        {
            capability_pointer = nullptr;
            state.local_data.fill(0);
            return 0;
        }

        // all child nodes are also revoked.
        common::error revoke_all()
        {
            return 0;
        }
    };

}

#endif
