#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include <stddef.h>
#include <stdint.h>

#include "memory_type.h"

#include <process.hpp>

namespace kernel
{
    constexpr static uint16_t PAGE_SIZE = 4096;

    struct memory_frame
    {
        process *owner;
        bool is_allocated;
    };

    struct memory_block
    {
        uint64_t physical_address;
        size_t size;
        memory_block *next;
        memory_frame memory_frames[1];
        uint64_t memory_frame_count;
    };

    // future: delegate physical_memory_allocation_system to virtual_memory_server (user-space).
    class memory_manager
    {
        public:
            memory_manager(const memory_info &target_memory_info);
            void *allocate_physical_memory(size_t size, process *owner);
            void deallocate_physical_memory(void *pointer);

        private:
            memory_block *head_memory_block;
            void init(const memory_info &target_memory_info);
            void init_memory_block(const memory_info &target_memory_info);
            void init_memory_frame(memory_block &target_memory_block);
            size_t align_size(size_t size, uint16_t page_size);
            uint64_t align_physical_address(uint64_t physical_address, uint16_t page_size);
            void configure_memory_frames(memory_frame *start_frame, uint64_t end_frame_index, process *owner, bool flag);
            memory_block *find_free_memory_block(size_t size);
            bool find_free_frames(memory_block &target_memory_block, uint64_t page_count, uint64_t &start_frame_index);
    };
}

#endif

