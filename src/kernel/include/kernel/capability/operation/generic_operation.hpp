#ifndef A9N_KERNEL_GENERIC_OPERATION_HPP
#define A9N_KERNEL_GENERIC_OPERATION_HPP

#include <kernel/types.hpp>

namespace a9n::kernel
{
    // buffer->tag
    enum class generic_operation : a9n::word
    {
        CONVERT = 0,
    };

    // buffer->get_message
    namespace generic_convert_argument
    {
        inline constexpr a9n::word CAPABILITY_TYPE      = 0;
        inline constexpr a9n::word CAPABILITY_SIZE_BITS = 1;
        inline constexpr a9n::word CAPABILITY_COUNT     = 2;
        inline constexpr a9n::word ROOT_DESCRIPTOR      = 3;
        inline constexpr a9n::word ROOT_DEPTH           = 4;
        inline constexpr a9n::word SLOT_INDEX           = 5;
    };
}

#endif
