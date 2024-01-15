#include <kernel/capability/generic.hpp>

#include <library/common/types.hpp>

namespace kernel
{
    generic::generic(
        common::physical_address initial_start_address,
        common::word initial_size,
        bool initial_flags
    )
        : start_address(initial_start_address)
        , size(initial_size)
        , flags(initial_flags)
        , watermark(0)
    {
    }

    common::error
        generic::execute(message_buffer *buffer, capability_entry *stored_entry)
    {
    }

    capability_entry *generic::traverse_entry(
        library::capability::capability_descriptor,
        common::word depth
    )
    {
        return nullptr;
    }
}
