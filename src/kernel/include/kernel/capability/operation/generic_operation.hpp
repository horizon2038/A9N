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
        // 0 is descriptor
        inline constexpr a9n::word OPERATION_TYPE           = 1; // tag
        inline constexpr a9n::word CAPABILITY_TYPE          = 2;
        inline constexpr a9n::word CAPABILITY_SPECIFIC_BITS = 3;
        inline constexpr a9n::word CAPABILITY_COUNT         = 4;
        inline constexpr a9n::word ROOT_DESCRIPTOR          = 5;
        inline constexpr a9n::word SLOT_INDEX               = 6;
    };
}

#endif
