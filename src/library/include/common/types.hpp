#ifndef TYPES_HPP
#define TYPES_HPP

#include "common.hpp"
#include <stdint.h>

namespace library::common
{
    // architecture-dependent size
    using word = uintmax_t;
    using sword = intmax_t;

    using virtual_address = word;
    using physical_address = word;

    using virtual_pointer = uintptr_t;
    using physical_pointer = uintptr_t;

    using error = sword;
}

#endif
