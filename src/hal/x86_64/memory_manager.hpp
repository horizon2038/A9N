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
            void virtual_memory_map() override;
            void virtual_memory_unmap() override;

        private:
            page_table page_table_level_1;
            page_table page_table_level_2;
            page_table page_table_level_3;
            page_table page_table_level_4;

    };
}

#endif
