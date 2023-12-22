#ifndef TYPES_HPP
#define TYPES_HPP

#include <stdint.h>

namespace library::common
{
    using word_t = uintmax_t;

    using virtual_address = word_t;
    using physical_address = word_t;

    using virtual_pointer = uintptr_t;
    using physical_pointer = uintptr_t;
}

#endif
