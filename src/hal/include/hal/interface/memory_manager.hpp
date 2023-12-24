#ifndef HAL_MEMORY_MANAGER_HPP
#define HAL_MEMORY_MANAGER_HPP

#include <stdint.h>
#include <stddef.h>
#include <kernel/process/process.hpp>
#include <library/common/types.hpp>

namespace hal::interface
{
    class memory_manager
    {
        public:
            virtual void init_memory() = 0;

            virtual void init_virtual_memory(
                common::physical_address top_page_table_address
            ) = 0;

            virtual bool is_table_exists(
                common::physical_address top_page_table,
                common::virtual_address target_virtual_address
            ) = 0;

            virtual void configure_page_table(
                common::physical_address top_page_table_address,
                common::virtual_address target_virtual_address,
                common::physical_address page_table_address
            ) = 0;

            virtual void map_virtual_memory(
                common::physical_address top_page_table_address,
                common::virtual_address target_virtual_addresss,
                common::physical_address target_physical_address
            ) = 0;

            virtual void unmap_virtual_memory(
                common::physical_address top_page_table_address,
                common::virtual_address target_virtual_address
            ) = 0;

            virtual common::virtual_address convert_physical_to_virtual_address(
                const common::physical_address target_physical_address
            ) = 0;

            virtual common::physical_address
                convert_virtual_to_physical_address(
                    const common::virtual_address target_virtual_address
                )
                = 0;
    };
}

#endif
