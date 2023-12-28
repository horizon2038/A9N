#ifndef CAPABILITY_NODE_HPP
#define CAPABILITY_NODE_HPP

#include <kernel/capability/capability.hpp>

#include <library/common/types.hpp>

namespace kernel
{
    // users never keep the address of capability_entry.
    // capability_descriptors are only used for indirect adressing.
    struct dependency_node;

    struct capability_entry
    {
        capability_data capability;
        dependency_node *family_node;
    };

    struct capability_node
    {
        common::word guard;
        common::word radix;
        capability_entry *entry;
    };

    struct dependency_node
    {
        // sibling capability_entry
        capability_entry *next_capability_entry;
        capability_entry *preview_capability_entry;

        // child capability_entry
        capability_entry *child_capability_entry;
    };
}

#endif
