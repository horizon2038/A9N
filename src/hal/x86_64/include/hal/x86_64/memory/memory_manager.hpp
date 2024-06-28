#ifndef X86_64_MEMORY_MANAGER_HPP
#define X86_64_MEMORY_MANAGER_HPP

#include <hal/interface/memory_manager.hpp>

#include <stdint.h>
#include <library/common/types.hpp>
#include "paging.hpp"

namespace hal::x86_64
{
    class memory_manager final : public hal::interface::memory_manager
    {
      public:
        void init_memory() override;

        void init_virtual_memory(common::physical_address top_page_table_address
        ) override;

        bool is_table_exists(
            common::physical_address top_page_table,
            common::virtual_address  target_virtual_address
        ) override;

        void configure_page_table(
            common::physical_address top_page_table_address,
            common::virtual_address  target_virtual_address,
            common::physical_address page_table_address
        ) override;

        void map_virtual_memory(
            common::physical_address top_page_table_address,
            common::virtual_address  target_virtual_address,
            common::physical_address target_physical_address
        ) override;

        void unmap_virtual_memory(
            common::physical_address top_page_table_address,
            common::virtual_address  target_virtual_address
        ) override;

        common::virtual_address convert_physical_to_virtual_address(
            const common::physical_address target_physical_address
        ) override;

        common::physical_address convert_virtual_to_physical_address(
            const common::virtual_address target_virtual_address
        ) override;

      private:
    };
}

#endif
