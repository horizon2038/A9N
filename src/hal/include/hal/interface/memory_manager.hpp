#ifndef HAL_MEMORY_MANAGER_HPP
#define HAL_MEMORY_MANAGER_HPP

#include <hal/hal_result.hpp>
#include <kernel/memory/memory_type.hpp>
#include <kernel/process/process.hpp>
#include <kernel/types.hpp>
#include <stddef.h>
#include <stdint.h>

namespace a9n::hal
{
    liba9n::result<a9n::kernel::page_table, hal_error>
        make_address_space(a9n::physical_address address);

    kernel::memory_map_result<> validate_root_address_space(const a9n::kernel::page_table &target_root
    );

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
