#include <kernel/capability/generic.hpp>

#include <library/common/types.hpp>

namespace kernel
{
    common::error generic::execute(capability_data data)
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
