#ifndef X86_64_MEMORY_MANAGER_HPP
#define X86_64_MEMORY_MANAGER_HPP

#include <interface/memory_manager.hpp>

#include <stdint.h>
#include "common.hpp"
#include "paging.hpp"

namespace hal::x86_64
{
    extern "C" uint64_t __kernel_pml4;

    class memory_manager final : public hal::interface::memory_manager
    {
        public:
            void init_memory() override;

            void init_virtual_memory(kernel::process *target_process) override;

            bool is_table_exists(kernel::physical_address top_page_table, kernel::virtual_address target_virtual_address) override;

            void configure_page_table
            (
                kernel::physical_address top_page_table_address,
                kernel::virtual_address target_virtual_address,
                kernel::physical_address page_table_address
            ) override;

            void map_virtual_memory
            (
                kernel::physical_address top_page_table_address,
                kernel::virtual_address target_virtual_address,
                kernel::physical_address target_physical_address,
                uint64_t page_count
            ) override;

            void unmap_virtual_memory
            (
                kernel::physical_address top_page_table_address,
                kernel::virtual_address target_virtual_address,
                uint64_t page_count
            ) override;

            kernel::virtual_address convert_physical_to_virtual_address
            (
                const kernel::physical_address target_physical_address
            ) override;

            kernel::physical_address convert_virtual_to_physical_address
            (
                const kernel::virtual_address target_virtual_address
            ) override;

        private:

    };
}

#endif
