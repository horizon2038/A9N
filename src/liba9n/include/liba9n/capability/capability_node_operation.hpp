#ifndef CAPABILITY_NODE_OPERATION_HPP
#define CAPABILITY_NODE_OPERATION_HPP

#include <kernel/types.hpp>

namespace liba9n::capability
{
    // since node_operation_type must be the same size as one entry of
    // message_buffer, it is specified as word size.
    enum class node_operation_type : a9n::word
    {
        COPY,
        MOVE,
        REVOKE,
        REMOVE,
    };
}

#endif
