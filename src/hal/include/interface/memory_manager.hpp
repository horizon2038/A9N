#ifndef HAL_MEMORY_MANAGER_HPP
#define HAL_MEMORY_MANAGER_HPP

#include <stdint.h>
#include <stddef.h>
#include <process.hpp>

namespace hal::interface
{
    class memory_manager
    {
        public:
            virtual void init_memory() = 0;
            virtual void init_virtual_memory(uint64_t target_page_table) = 0;
            virtual void map_virtual_memory
            (
                kernel::process *target_process,
                uint64_t virtual_addresss,
                uint64_t physical_address,
                uint64_t page_count
            ) = 0;
            virtual void unmap_virtual_memory
            (
                kernel::process *target_process,
                uint64_t virtual_address,
                uint64_t page_count
            ) = 0;

            /*
            TODO: create memory-management interface and 
            impl of arch-specific interface implement.
            - allocate physical-memory
            - map virtual-memory
            - unmap virtual-memory
            - initialize page-table
                - arch-specific: void pointer -> cast ?
            */
    };
}

#endif
