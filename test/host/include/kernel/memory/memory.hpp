#pragma once
#include <kernel/types.hpp>

// Host page tables use real host pointers as simulated physical addresses.
// Frame physical addresses are only written into PTEs, never dereferenced.
namespace a9n::kernel
{
    template<typename T>
    inline T *physical_to_virtual_pointer(a9n::physical_address address)
    {
        return reinterpret_cast<T *>(address);
    }
    inline a9n::virtual_address physical_to_virtual_address(a9n::physical_address address)
    {
        return address;
    }
    inline a9n::physical_address virtual_to_physical_address(a9n::virtual_address address)
    {
        return address;
    }
}
