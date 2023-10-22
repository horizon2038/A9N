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
            void map_virtual_memory
            (
                kernel::process *target_process,
                kernel::virtual_address target_virtual_address,
                kernel::physical_address target_physical_address,
                uint64_t page_count
            ) override;
            void unmap_virtual_memory
            (
                kernel::process *target_process,
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
            void map_page(page_table &top_page_table, uint64_t virtual_address, uint64_t physical_address);
            page_table *create_page_tabe(page_table* parent_page_table, uint64_t index);
            page_table *acquire_page_table(page_table &parent_page_table, uint64_t index);

    };
}

#endif
