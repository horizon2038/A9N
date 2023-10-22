#ifndef HAL_MEMORY_MANAGER_HPP
#define HAL_MEMORY_MANAGER_HPP

#include <stdint.h>
#include <stddef.h>
#include <process.hpp>
#include <common.hpp>

namespace hal::interface
{
    class memory_manager
    {
        public:
            virtual void init_memory() = 0;
            virtual void init_virtual_memory(kernel::process *target_process) = 0;
            virtual void map_virtual_memory
            (
                kernel::process *target_process,
                kernel::virtual_address target_virtual_addresss,
                kernel::physical_address target_physical_address,
                uint64_t page_count
            ) = 0;
            virtual void unmap_virtual_memory
            (
                kernel::process *target_process,
                kernel::virtual_address target_virtual_address,
                uint64_t page_count
            ) = 0;
            virtual kernel::virtual_address convert_physical_to_virtual_address
            (
                const kernel::physical_address target_physical_address
            ) = 0;

            virtual kernel::physical_address convert_virtual_to_physical_address
            (
                const kernel::virtual_address target_virtual_address
            ) = 0;
            /*
            TODO: create memory-management interface and 
            impl of arch-specific interface implement.
            - map virtual-memory
            - unmap virtual-memory
            - initialize page-table
                - arch-specific: void pointer -> cast ?
            */
    };
}

#endif
