#ifndef HAL_MEMORY_MANAGER_HPP
#define HAL_MEMORY_MANAGER_HPP

#include <kernel/process/process.hpp>
#include <kernel/types.hpp>
#include <stddef.h>
#include <stdint.h>

namespace a9n::hal
{
    class memory_manager
    {
      public:
        virtual void init_memory() = 0;

        virtual void init_virtual_memory(a9n::physical_address top_page_table_address
        )                          = 0;

        virtual bool is_table_exists(
            a9n::physical_address top_page_table,
            a9n::virtual_address  target_virtual_address
        ) = 0;

        virtual void configure_page_table(
            a9n::physical_address top_page_table_address,
            a9n::virtual_address  target_virtual_address,
            a9n::physical_address page_table_address
        ) = 0;

        virtual void map_virtual_memory(
            a9n::physical_address top_page_table_address,
            a9n::virtual_address  target_virtual_addresss,
            a9n::physical_address target_physical_address
        ) = 0;

        virtual void unmap_virtual_memory(
            a9n::physical_address top_page_table_address,
            a9n::virtual_address  target_virtual_address
        ) = 0;

        virtual a9n::virtual_address convert_physical_to_virtual_address(
            const a9n::physical_address target_physical_address
        ) = 0;

        virtual a9n::physical_address convert_virtual_to_physical_address(
            const a9n::virtual_address target_virtual_address
        ) = 0;
    };
}

#endif
