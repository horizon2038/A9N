#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include <stddef.h>
#include <stdint.h>

#include "memory_type.h"

namespace kernel
{
    constexpr static uint16_t PAGE_SIZE = 4096;

    struct memory_block
    {
        uint64_t physical_address;
        size_t size;
        memory_block *next;
    };

    // future: delegate physical_memory_allocation_system to virtual_memory_server (user-space).
    class memory_manager
    {
        public:
            memory_manager(const memory_info &target_memory_info);
            void *allocate_physical_memory(size_t size);
            void dealocate_physical_memory(void *pointer);

        private:
            memory_block *head_memory_block;
            void init(const memory_info &target_memory_info);
            void init_memory_block(const memory_info &target_memory_info);
            size_t align_size(size_t size, uint16_t page_size);
            uint64_t align_physical_address(uint64_t physical_address, uint16_t page_size);
    };
}

#endif

