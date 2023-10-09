#ifndef X86_64_MEMORY_MANAGER_HPP
#define X86_64_MEMORY_MANAGER_HPP

#include <interface/memory_manager.hpp>

#include <stdint.h>
#include "paging.hpp"

namespace hal::x86_64
{
    class memory_manager final : public hal::interface::memory_manager
    {
        public:
            void init_memory() override;
            void init_page_table(uint64_t target_page_table) override;
            void virtual_memory_map
            (
                kernel::process *target_process,
                uint64_t virtual_addresss,
                uint64_t physical_address
            ) override;
            void virtual_memory_unmap() override;

        private:
            page_table page_table_level_1;

    };
}

#endif
