#include "memory_manager.hpp"

#include <cpp_dependent/new.hpp>

namespace kernel
{
    memory_manager::memory_manager(const memory_info &target_memory_info)
    : head_memory_block(nullptr)
    {
        init(target_memory_info);
    }

    void memory_manager::init(const memory_info &target_memory_info)
    {
        /*
        TODO: allocate_physical_memoryを実装する.
        memory_infoからmemory_blockへ変換する.
        allocateしてからmemory_blockをそこへ配置し, 適切に設定する.
        */
        init_memory_block(target_memory_info);
    }

    void memory_manager::init_memory_block(const memory_info &target_memory_info)
    {
        /*
        memory_block is like a header placed at the beginning of free memory.
        therefore, memory allocations are not required
        (since it is assumed that free memory exists).
        */
        memory_map_entry *_memory_map_entry = nullptr;
        memory_block *previous_memory_block = nullptr;

        for (uint16_t i = 0; i < target_memory_info.memory_map_count; i++)
        {
            _memory_map_entry = &target_memory_info.memory_map[i];


            if (!(_memory_map_entry->is_free))
            {
                continue;
            }

            uint64_t memory_block_size = sizeof(memory_block) + sizeof(memory_frame) * (_memory_map_entry->page_count - 1);
            uint64_t adjusted_address = _memory_map_entry->physical_address_start + memory_block_size;
            size_t adjusted_size = PAGE_SIZE * (_memory_map_entry->page_count - 1) - memory_block_size;

            memory_block *current_memory_block = reinterpret_cast<memory_block*>(_memory_map_entry->physical_address_start); 
            current_memory_block->physical_address = adjusted_address;
            current_memory_block->size = adjusted_size;
            current_memory_block->next = nullptr;
            current_memory_block->memory_frame_count = adjusted_size / PAGE_SIZE;

            init_memory_frame(*current_memory_block);

            if (!previous_memory_block)
            {
                head_memory_block = current_memory_block;
                previous_memory_block = current_memory_block;
                continue;
            }
            previous_memory_block->next = current_memory_block;
            previous_memory_block = current_memory_block;
        }
    }

    void memory_manager::init_memory_frame(memory_block &target_memory_block)
    {
            for (uint64_t j = 0; j < target_memory_block.memory_frame_count - 1; j++)
            {
                target_memory_block.memory_frames[j].owner = nullptr;
                target_memory_block.memory_frames[j].is_allocated = false;
            }
    }

    void *memory_manager::allocate_physical_memory(size_t size)
    {
        size_t aligned_size = align_size(size, PAGE_SIZE); 
        memory_block *current_memory_block = head_memory_block;
        memory_block *previous_memory_block = nullptr;

        while (current_memory_block)
        {
            if (current_memory_block->size >= aligned_size)
            {
                current_memory_block->size -= size;
                return reinterpret_cast<void*>(current_memory_block->physical_address);
            }
            previous_memory_block = current_memory_block;
            current_memory_block = current_memory_block->next;
        }
        return nullptr;
    }

    void memory_manager::dealocate_physical_memory(void *pointer)
    {
    }

    size_t memory_manager::align_size(size_t size, uint16_t page_size)
    {
        size_t aligned_size = (size + page_size - 1) & ~(page_size - 1);
        return aligned_size;
    }

    uint64_t memory_manager::align_physical_address(uint64_t physical_address, uint16_t page_size)
    {
        uint64_t aligned_address = (physical_address + page_size - 1) & ~(page_size - 1); 
        return aligned_address;
    }

}
