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

        constexpr inline a9n::word CAPABILITY_TYPE      = 3;
        constexpr inline a9n::word CAPABILITY_SIZE_BITS = 4;
        constexpr inline a9n::word CAPABILITY_COUNT     = 5;
        constexpr inline a9n::word ROOT_DESCRIPTOR      = 6;
        constexpr inline a9n::word ROOT_DEPTH           = 7;
        constexpr inline a9n::word SLOT_INDEX           = 8;
    };
}

#endif
