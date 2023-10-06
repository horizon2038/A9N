#ifndef HAL_MEMORY_MANAGER_HPP
#define HAL_MEMORY_MANAGER_HPP

#include <stdint.h>

namespace hal::interface
{
    class memory_manager
    {
        public:
            virtual void init_memory(uint64_t target_page_table) = 0;
            virtual void virtual_memory_map() = 0;
            virtual void virtual_memory_unmap() = 0;
    };
}

#endif
