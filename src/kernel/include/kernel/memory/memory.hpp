#ifndef A9N_KERNEL_MEMORY_HPP
#define A9N_KERNEL_MEMORY_HPP

#include <hal/arch/arch_types.hpp>
#include <kernel/types.hpp>

namespace a9n::kernel
{
    template<typename T>
    inline T *physical_to_virtual_pointer(a9n::physical_address target_physical_address)
    {
        return reinterpret_cast<T *>(target_physical_address | a9n::hal::KERNEL_VIRTUAL_BASE);
    }

    inline a9n::virtual_address
        physical_to_virtual_address(a9n::physical_address target_physical_address)
    {
        return reinterpret_cast<a9n::virtual_address>(
            target_physical_address | a9n::hal::KERNEL_VIRTUAL_BASE
        );
    }

    // WARN: it is a very dengerous function !; use with caution!
    template<typename T>
    [[deprecated("virtual_to_physical_pointer<T> : unsafe and not recommended")]]
    inline T *virtual_to_physical_pointer(a9n::virtual_address kernel_virtual_address)
    {
        return reinterpret_cast<T *>(kernel_virtual_address & ~a9n::hal::KERNEL_VIRTUAL_BASE);
    }

    inline a9n::physical_address
        virtual_to_physical_address(a9n::virtual_address kernel_virtual_address)
    {
        return reinterpret_cast<a9n::virtual_address>(
            kernel_virtual_address & ~a9n::hal::KERNEL_VIRTUAL_BASE
        );
    }

}

#endif
