#ifndef TYPES_HPP
#define TYPES_HPP

#include <stdint.h>

#ifdef __cplusplus

// for C++
namespace a9n
{
    // architecture-dependent size
    using word                      = uintmax_t;
    using sword                     = intmax_t;

    static constexpr word BYTE_BITS = 8;
    static constexpr word WORD_BITS = sizeof(word) * BYTE_BITS;

    using virtual_address           = word;
    using physical_address          = word;

    using virtual_pointer           = uintptr_t;
    using physical_pointer          = uintptr_t;

    using error                     = sword;

    static constexpr word PAGE_SIZE = 4096;

    using capability_descriptor     = word;

    inline constexpr word extract_depth(capability_descriptor descriptor)
    {
        constexpr word BYTE_MASK = (static_cast<a9n::word>(1) << a9n::BYTE_BITS) - 1;
        return ((descriptor >> (a9n::WORD_BITS - a9n::BYTE_BITS)) & BYTE_MASK) + a9n::BYTE_BITS;
    }
}

#else

// for C
typedef uintmax_t word;
typedef intmax_t  sword;

typedef word      virtual_address;
typedef word      physical_address;
typedef uintptr_t virtual_pointer;
typedef uintptr_t physical_pointer;

typedef error = sword;

#endif

#endif
