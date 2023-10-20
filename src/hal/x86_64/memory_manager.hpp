#ifndef X86_64_MEMORY_MANAGER_HPP
#define X86_64_MEMORY_MANAGER_HPP

#include <interface/memory_manager.hpp>

#include <stdint.h>
#include "paging.hpp"

namespace hal::x86_64
{
    constexpr uint16_t PAGE_SIZE = 4096;

    class memory_manager final : public hal::interface::memory_manager
    {
        public:
            void init_memory() override;
            void init_virtual_memory(uint64_t top_page_table) override;
            void map_virtual_memory
            (
                kernel::process *target_process,
                uint64_t virtual_addresss,
                uint64_t physical_address,
                uint64_t page_count
            ) override;
            void unmap_virtual_memory
            (
                kernel::process *target_process,
                uint64_t virtual_address,
                uint64_t page_count
            ) override;
            

        private:
            page_table kernel_top_page_table;

            void map_page(page_table &top_page_table, uint64_t virtual_address, uint64_t physical_address);
            page_table *create_page_tabe(page_table* parent_page_table, uint64_t index);
            page_table *acquire_page_table(page_table &parent_page_table, uint64_t index);

    };
}

#endif
