#ifndef COMMON_HPP
#define COMMON_HPP

#include <stdint.h>

namespace kernel
{
    // constant
    constexpr static uint16_t PAGE_SIZE = 4096;

    // type definition
    using virtual_address = uint64_t;
    using physical_address = uint64_t;
}

#endif
