#ifndef HAL_MEMORY_MANAGER_HPP
#define HAL_MEMORY_MANAGER_HPP

#include <hal/hal_result.hpp>
#include <kernel/memory/memory_type.hpp>
#include <kernel/process/process.hpp>
#include <kernel/types.hpp>
#include <stddef.h>
#include <stdint.h>

namespace a9n::hal::x86_64
{
    hal_result unmap_lower_memory_mapping(void);
}

namespace a9n::hal
{
    class memory_manager
    {
      public:
        virtual void init_memory()                                                     = 0;

        virtual void init_virtual_memory(a9n::physical_address top_page_table_address) = 0;

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

    liba9n::result<a9n::kernel::page_table, hal_error>
        make_root_page_table(a9n::physical_address address);

    kernel::memory_map_result<> map_page_table(
        const a9n::kernel::page_table &target_root,
        const a9n::kernel::page_table &target_table,
        const a9n::virtual_address     target_address
    );
    kernel::memory_map_result<> unmap_page_table(
        const a9n::kernel::page_table &target_root,
        const a9n::kernel::page_table &target_table,
        const a9n::virtual_address     target_address
    );

    kernel::memory_map_result<> map_frame(
        const a9n::kernel::page_table &target_root,
        const a9n::kernel::frame      &target_frame,
        const a9n::virtual_address     target_address
    );
    kernel::memory_map_result<> unmap_frame(
        const a9n::kernel::page_table &target_root,
        const a9n::kernel::frame      &target_frame,
        const a9n::virtual_address     target_address
    );

    kernel::memory_map_result<a9n::word> search_unset_page_table_depth(
        const a9n::kernel::page_table &top_page_table_address,
        const a9n::virtual_address     target_address
    );

}

#endif
