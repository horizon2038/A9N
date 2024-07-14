#ifndef GENERIC_OPERATION_HPP
#define GENERIC_OPERATION_HPP

#include <kernel/types.hpp>

namespace liba9n::capability
{
    // generic.execute(CONVERT, type, size, descriptor, descriptor_depth,
    // descriptor_index);
    enum class generic_operation : a9n::word
    {
        CONVERT,
    };

    namespace convert_argument
    {
        // 0 : descriptor (this) [reserved]
        // 1 : descriptor_depth  [reserved]
        // 2 : operation_type
        // 3 : type
        // 4 : size
        // 5 : count
        // 6 : target node descriptor
        // 7 : target node depth
        // 8 : target node index

        inline constexpr a9n::word CAPABILITY_TYPE      = 0;
        inline constexpr a9n::word CAPABILITY_SIZE_BITS = 1;
        inline constexpr a9n::word CAPABILITY_COUNT     = 2;
        inline constexpr a9n::word ROOT_DESCRIPTOR      = 3;
        inline constexpr a9n::word ROOT_DEPTH           = 4;
        inline constexpr a9n::word SLOT_INDEX           = 5;
    };
}

#endif
