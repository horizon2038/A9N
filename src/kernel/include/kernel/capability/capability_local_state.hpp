#ifndef CAPABILITY_LOCAL_STATE_HPP
#define CAPABILITY_LOCAL_STATE_HPP

#include <liba9n/common/array.hpp>

namespace a9n::kernel
{
    constexpr static a9n::word ENTRY_DATA_MAX = 3;

    using capability_slot_data
        = liba9n::common::bounded_array<a9n::word, ENTRY_DATA_MAX>;

    struct dependency_node
    {
        // sibling capability_entry
        struct capability_slot *next_slot;
        struct capability_slot *preview_slot;

        // child capability_entry
        // capability_entry *child_capability_entry;
        a9n::word depth;
        // visitor ?
    };

}

#endif
