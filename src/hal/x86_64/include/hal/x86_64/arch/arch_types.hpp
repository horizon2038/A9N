#ifndef X86_64_COMMON_HPP
#define X86_64_COMMON_HPP

#include <stdint.h>
#include <liba9n/common/types.hpp>

namespace a9n::hal::x86_64
{
    // constant
    constexpr static uint64_t KERNEL_VIRTUAL_BASE = 0xFFFF800000000000;

    // common

    // these memory address translation variables assume that the kernel virtual
    // address is KERNEL_VIRTUAL_ADDRESS + physical_address.

    static inline a9n::virtual_address convert_physical_to_virtual_address(
        const a9n::physical_address target_physical_address
    )
    {
        return target_physical_address | KERNEL_VIRTUAL_BASE;
    }

    static inline a9n::physical_address convert_virtual_to_physical_address(
        const a9n::virtual_address target_virtual_address
    )
    {
        return target_virtual_address & ~KERNEL_VIRTUAL_BASE;
    }

    static inline bool
        is_canonical(const a9n::virtual_address target_virtual_address)
    {
        const uint64_t canonical_sign_bit   = target_virtual_address >> 47 & 1;
        const uint64_t canonical_upper_bits = target_virtual_address >> 48;

        return (
            (canonical_sign_bit == 0 && canonical_upper_bits == 0)
            || (canonical_sign_bit == 1 && canonical_upper_bits == 0xFFFF)
        );
    }

    template<typename T>
    T *convert_physical_to_virtual_pointer(a9n::physical_address address)
    {
        return reinterpret_cast<T *>(convert_physical_to_virtual_address(address
        ));
    }
}

#endif
