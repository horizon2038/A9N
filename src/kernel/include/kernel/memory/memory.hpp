#ifndef A9N_KERNEL_MEMORY_HPP
#define A9N_KERNEL_MEMORY_HPP

#include <kernel/types.hpp>
#include <hal/arch/arch_types.hpp>

namespace a9n::kernel
{
    template<typename T>
    T *physical_to_virtual_pointer(a9n::physical_address target_physical_address
    )
    {
        return reinterpret_cast<T *>(
            target_physical_address | a9n::hal::KERNEL_VIRTUAL_BASE
        );
    }

    template<typename T>
    a9n::virtual_address physical_to_virtual_address(
        a9n::physical_address target_physical_address
    )
    {
        return reinterpret_cast<a9n::virtual_address>(
            target_physical_address | a9n::hal::KERNEL_VIRTUAL_BASE
        );
    }

}

#endif
