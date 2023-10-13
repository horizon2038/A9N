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
        memory_map_entry *_memory_map_entry;
        memory_block *previous_memory_block = nullptr;

        for (uint16_t i = 0; i < target_memory_info.memory_map_count; i++)
        {
            _memory_map_entry = &target_memory_info.memory_map[i];

            if (!(_memory_map_entry->is_free))
            {
                continue;
            }

            memory_block *current_memory_block = reinterpret_cast<memory_block*>(_memory_map_entry->physical_address_start); 
            current_memory_block->physical_address = _memory_map_entry->physical_address_start + sizeof(memory_block);
            current_memory_block->size = PAGE_SIZE * (_memory_map_entry->page_count - 1);
            current_memory_block->next = nullptr;

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

    void *memory_manager::allocate_physical_memory(size_t size)
    {
        size_t aligned_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
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
}
