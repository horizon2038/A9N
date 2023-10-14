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
        for (uint64_t i = 0; i < target_memory_block.memory_frame_count; i++)
        {
            target_memory_block.memory_frames[i].owner = nullptr;
            target_memory_block.memory_frames[i].is_allocated = false;
        }
    }

    void *memory_manager::allocate_physical_memory(size_t size, process *owner)
    {
        size_t aligned_requested_size = align_size(size, PAGE_SIZE); 
        size_t requested_page_count = aligned_requested_size / PAGE_SIZE;
        memory_block *current_memory_block;
        uint64_t start_frame_index;
        bool has_free_frames;

        while((current_memory_block = find_free_memory_block(aligned_requested_size)))
        {
            has_free_frames = find_free_frames(*current_memory_block, requested_page_count, start_frame_index);
            
            if (has_free_frames)
            {
                configure_memory_frames(&current_memory_block->memory_frames[start_frame_index], requested_page_count, owner, true);
                uint64_t start_frame_address = current_memory_block->physical_address + (start_frame_index * PAGE_SIZE);
                return reinterpret_cast<void*>(start_frame_address);
            }

            current_memory_block->size = 0;
        }

        return nullptr;
    }

    memory_block *memory_manager::find_free_memory_block(size_t size)
    {
        memory_block *current_memory_block = head_memory_block;

        while (current_memory_block)
        {
            if (current_memory_block->size >= size)
            {
                return current_memory_block;
            }
            current_memory_block = current_memory_block->next;
        }

        return nullptr;
    }

    bool memory_manager::find_free_frames
    (
        memory_block &target_memory_block,
        uint64_t page_count,
        uint64_t &start_frame_index
    )
    {
        uint64_t free_page_count = 0;

        for (uint64_t i = 0; i < target_memory_block.memory_frame_count; i++)
        {
            if (target_memory_block.memory_frames[i].is_allocated)
            {
                free_page_count = 0;
                continue;
            }

            free_page_count++;

            if (free_page_count == page_count)
            {
                start_frame_index = i + 1 - free_page_count;
                return true;
            }
        }
        return false;
    }

    void memory_manager::configure_memory_frames(memory_frame *start_frame, uint64_t page_count, process *owner, bool flag)
    {
        for (uint64_t i = 0; i < page_count; i++)
        {
            start_frame[i].is_allocated = flag;
            start_frame[i].owner = owner;
        }
    }

    void memory_manager::deallocate_physical_memory(void *pointer)
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
