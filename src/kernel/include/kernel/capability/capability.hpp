#ifndef CAPABILITY_HPP
#define CAPABILITY_HPP

#include <kernel/ipc/message_buffer.hpp>
#include <library/capability/capability_descriptor.hpp>
#include <library/common/types.hpp>
#include <library/common/array.hpp>

namespace kernel
{
    enum class capability_type : uint8_t
    {
        // NODE type represents a node that can store capabilities itself.
        // They are used for exploration and are casted to node.
        NODE = 0x00,

        // REAL type represents objects that have a separate instance created
        // in memory, such as process_control_block.
        REAL = 0x01,

        // VIRTUAL type represents objects that do not have an instance created
        // in memory, such as frame or generic.
        // it can also be used for debugging capabilities.
        VIRTUAL = 0x02,
    };

    // capability_data is an essential presence for capability.execute().
    // This enables providing a common interface for capabilities such as
    // generic, frame, and others where we do not want to have a physical entity
    // in memory for each. This enhances extensibility and maintainability.

    // TODO: create operation structure (utilizes capcall argments)
    using entry_data = library::common::bounded_array<common::word, 4>;

    struct capability_entry;

    class capability
    {
      public:
        virtual common::error
            execute(message_buffer *buffer, capability_entry *stored_entry)
            = 0;
        virtual capability_entry *traverse_entry(
            library::capability::capability_descriptor descriptor,
            common::word depth
        ) = 0;
    };

    struct dependency_node
    {
        // sibling capability_entry
        capability_entry *next_capability_entry;
        capability_entry *preview_capability_entry;

        // child capability_entry
        capability_entry *child_capability_entry;
    };

    struct capability_entry
    {
        capability *capability_pointer;
        entry_data local_data;
        dependency_node family_node;

        common::error execute(message_buffer *buffer)
        {
            return capability_pointer->execute(buffer, this);
        }
    };

}

#endif
