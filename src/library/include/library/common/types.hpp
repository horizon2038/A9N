#ifndef TYPES_HPP
#define TYPES_HPP

#include <stdint.h>

#ifdef __cplusplus

// for C++
namespace library::common
{
    // architecture-dependent size
    using word = uintmax_t;
    using sword = intmax_t;

    constexpr static word BYTE_BITS = 8;
    constexpr static word WORD_BITS = sizeof(word) * BYTE_BITS;

    using virtual_address = word;
    using physical_address = word;

    using virtual_pointer = uintptr_t;
    using physical_pointer = uintptr_t;

    using error = sword;

    constexpr static word PAGE_SIZE = 4096;
}

namespace kernel
{
    namespace common = library::common;
}

namespace hal
{
    namespace common = library::common;
}

#else

// for C
typedef uintmax_t word;
typedef intmax_t sword;

typedef word virtual_address;
typedef word physical_address;
typedef uintptr_t virtual_pointer;
typedef uintptr_t physical_pointer;

typedef error = sword;

#endif

#endif
