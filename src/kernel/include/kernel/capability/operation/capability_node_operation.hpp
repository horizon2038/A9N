#ifndef A9N_KERNEL_CAPABILITY_NODE_OPERATION_HPP
#define A9N_KERNEL_CAPABILITY_NODE_OPERATION_HPP

#include <kernel/types.hpp>

namespace a9n::kernel
{
    // buffer->tag
    enum class capability_node_operation : a9n::word
    {
        COPY   = 0,
        MOVE   = 1,
        REMOVE = 2,
        REVOKE = 3,
    };

    // buffer->get_message
    // to avoid "premature commonization",
    // they are defined separately.
    namespace capability_node_copy_argument
    {
        inline constexpr a9n::word DESTINATION_INDEX = 0;
        inline constexpr a9n::word SOURCE_DESCRIPTOR = 1;
        inline constexpr a9n::word SOURCE_DEPTH      = 2;
        inline constexpr a9n::word SOURCE_INDEX      = 3;
    };

    namespace capability_node_move_argument
    {
        inline constexpr a9n::word DESTINATION_INDEX = 0;
        inline constexpr a9n::word SOURCE_DESCRIPTOR = 1;
        inline constexpr a9n::word SOURCE_DEPTH      = 2;
        inline constexpr a9n::word SOURCE_INDEX      = 3;
    };

    namespace capability_node_revoke_argument
    {
        inline constexpr a9n::word DESTINATION_INDEX = 0;
    };

    namespace capability_node_remove_argument
    {
        inline constexpr a9n::word DESTINATION_INDEX = 0;
    };
}

#endif
