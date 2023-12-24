#ifndef COMMON_HPP
#define COMMON_HPP

#include <stdint.h>

namespace kernel
{
    constexpr static uint16_t PAGE_SIZE = 4096;

    using virtual_address = uintmax_t;
    using physical_address = uintmax_t;

    using word_t = uintmax_t;

    using result = int;
    constexpr static result OK = 0;
    constexpr static result INVALID_ARGUMENT = -1;
    constexpr static result PERMISSION_DENIED = -2;
    
}

#endif
