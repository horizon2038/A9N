#ifndef CAPABILITY_LOCAL_STATE_HPP
#define CAPABILITY_LOCAL_STATE_HPP

#include <library/common/array.hpp>

namespace kernel
{
    constexpr static common::word ENTRY_DATA_MAX = 3;

    using capability_slot_data
        = library::common::bounded_array<common::word, ENTRY_DATA_MAX>;

    struct dependency_node
    {
        // sibling capability_entry
        struct capability_slot *next_slot;
        struct capability_slot *preview_slot;

        // child capability_entry
        // capability_entry *child_capability_entry;
        common::word depth;
        // visitor ?
    };

}

#endif
