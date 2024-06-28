#ifndef X86_64_MEMORY_MANAGER_HPP
#define X86_64_MEMORY_MANAGER_HPP

#include <hal/interface/memory_manager.hpp>

#include <stdint.h>
#include <kernel/types.hpp>
#include "paging.hpp"

namespace a9n::hal::x86_64
{
    class memory_manager final : public a9n::hal::memory_manager
    {
      public:
        void init_memory() override;

        void init_virtual_memory(a9n::physical_address top_page_table_address
        ) override;

        bool is_table_exists(
            a9n::physical_address top_page_table,
            a9n::virtual_address  target_virtual_address
        ) override;

        void configure_page_table(
            a9n::physical_address top_page_table_address,
            a9n::virtual_address  target_virtual_address,
            a9n::physical_address page_table_address
        ) override;

        void map_virtual_memory(
            a9n::physical_address top_page_table_address,
            a9n::virtual_address  target_virtual_address,
            a9n::physical_address target_physical_address
        ) override;

        void unmap_virtual_memory(
            a9n::physical_address top_page_table_address,
            a9n::virtual_address  target_virtual_address
        ) override;

        a9n::virtual_address convert_physical_to_virtual_address(
            const a9n::physical_address target_physical_address
        ) override;

        a9n::physical_address convert_virtual_to_physical_address(
            const a9n::virtual_address target_virtual_address
        ) override;

      private:
    };
}

#endif
