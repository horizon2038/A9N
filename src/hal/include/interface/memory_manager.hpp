#ifndef HAL_MEMORY_MANAGER_HPP
#define HAL_MEMORY_MANAGER_HPP

#include <stdint.h>
#include <stddef.h>
#include <process/process.hpp>
#include <common.hpp>

namespace hal::interface
{
    class memory_manager
    {
        public:
            virtual void init_memory() = 0;

            virtual void init_virtual_memory(kernel::physical_address top_page_table_address) = 0;

            virtual bool is_table_exists(kernel::physical_address top_page_table, kernel::virtual_address target_virtual_address) = 0;

            virtual void configure_page_table
            (
                kernel::physical_address top_page_table_address,
                kernel::virtual_address target_virtual_address,
                kernel::physical_address page_table_address
            ) = 0;

            virtual void map_virtual_memory
            (
                kernel::physical_address top_page_table_address,
                kernel::virtual_address target_virtual_addresss,
                kernel::physical_address target_physical_address
            ) = 0;

            virtual void unmap_virtual_memory
            (
                kernel::physical_address top_page_table_address,
                kernel::virtual_address target_virtual_address
            ) = 0;

            virtual kernel::virtual_address convert_physical_to_virtual_address
            (
                const kernel::physical_address target_physical_address
            ) = 0;

            virtual kernel::physical_address convert_virtual_to_physical_address
            (
                const kernel::virtual_address target_virtual_address
            ) = 0;
    };
}

#endif
