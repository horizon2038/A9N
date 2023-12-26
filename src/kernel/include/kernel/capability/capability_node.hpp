#ifndef CAPABILITY_NODE_HPP
#define CAPABILITY_NODE_HPP

#include <kernel/capability/capability.hpp>

#include <library/common/types.hpp>

namespace kernel
{
    // users never keep the address of capability_entry.
    // capability_descriptors are only used for indirect adressing.
    struct capability_entry
    {
        common::virtual_address capability_address;
    };

    struct capability_node
    {
    };
}

#endif
