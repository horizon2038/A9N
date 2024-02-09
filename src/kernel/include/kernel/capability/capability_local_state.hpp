#ifndef CAPABILITY_LOCAL_STATE_HPP
#define CAPABILITY_LOCAL_STATE_HPP

#include <library/common/array.hpp>

namespace kernel
{
    constexpr static common::word ENTRY_DATA_MAX = 3;

    using capability_entry_data
        = library::common::bounded_array<common::word, ENTRY_DATA_MAX>;

    struct dependency_node
    {
        // sibling capability_entry
        struct capability_entry *next_capability_entry;
        struct capability_entry *preview_capability_entry;

        // child capability_entry
        // capability_entry *child_capability_entry;
        common::word depth;
        // visitor ?
    };

    struct capability_local_state
    {
        capability_entry_data data;
        dependency_node family_node;
    };

}

#endif
