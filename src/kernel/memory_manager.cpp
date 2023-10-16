#include "memory_manager.hpp"

#include <library/logger.hpp>

namespace kernel
{
    memory_manager::memory_manager(const memory_info &target_memory_info)
    : head_memory_block(nullptr)
    {
        init(target_memory_info);
    }

    void memory_manager::init(const memory_info &target_memory_info)
    {
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
            size_t adjusted_size = PAGE_SIZE * (_memory_map_entry->page_count) - memory_block_size;

            memory_block *current_memory_block = reinterpret_cast<memory_block*>(_memory_map_entry->physical_address_start); 
            current_memory_block->physical_address = adjusted_address;
            current_memory_block->size = adjusted_size;
            current_memory_block->next = nullptr;
            current_memory_block->memory_frame_count = adjusted_size / PAGE_SIZE;


            init_memory_frame(*current_memory_block);

            if (head_memory_block == nullptr)
            {
                kernel::utility::logger::printk("init_first!\n");
                kernel::utility::logger::split();
                head_memory_block = current_memory_block;
            }
            else
            {
                if (previous_memory_block != nullptr)
                {
                    previous_memory_block->next = current_memory_block;
                }
            }

            print_memory_block_info(*current_memory_block);
            previous_memory_block = current_memory_block;

        }
        memory_block *now_memory_block = head_memory_block;

        while (now_memory_block)
        {
            kernel::utility::logger::debug("test_memory_block_init");
            print_memory_block_info(*now_memory_block);
            now_memory_block = now_memory_block->next;
        }
    }

    void memory_manager::print_memory_block_info(memory_block &target_memory_block)
    {
        using logger = kernel::utility::logger;

        uint64_t memory_map_size = target_memory_block.memory_frame_count * PAGE_SIZE;
        uint64_t memory_map_size_kb = memory_map_size / 1024;
        uint64_t memory_map_size_mb = memory_map_size_kb / 1024;

        logger::printk("----- memory_block_info\e[60G%16s\n", "-----");
        logger::printk("physical_address\e[52G:\e[58G0x%016llx\n", target_memory_block.physical_address); 
        logger::printk("next_physical_address\e[52G:\e[58G0x%016llx\n", target_memory_block.next->physical_address); 
        logger::printk
        (
            "size\e[52G:\e[60G%16llu B | %6llu KiB | %6llu MiB \n",
            memory_map_size,
            memory_map_size_kb,
            memory_map_size_mb
        ); 
        logger::printk("memory_frame_count\e[52G:\e[60G%16llu x 4KiB\n", target_memory_block.memory_frame_count); 
        logger::split();
    }

    void memory_manager::init_memory_frame(memory_block &target_memory_block)
    {
        kernel::utility::logger::printk("init_memory_frame\n"); 
        for (uint64_t i = 0; i < target_memory_block.memory_frame_count; i++)
        {
            target_memory_block.memory_frames[i].owner = nullptr;
            target_memory_block.memory_frames[i].is_allocated = false;
        }
        kernel::utility::logger::split();
    }

    void *memory_manager::allocate_physical_memory(size_t size, process *owner)
    {
        size_t aligned_requested_size = align_size(size, PAGE_SIZE); 
        size_t requested_page_count = aligned_requested_size / PAGE_SIZE;
        memory_block *current_memory_block = head_memory_block;
        kernel::utility::logger::printk("memory_frame_count_head: %llu\n", current_memory_block->memory_frame_count);
        uint64_t start_frame_index;
        bool has_free_frames;

        while(current_memory_block)
        {
            has_free_frames = find_free_frames(*current_memory_block, requested_page_count, start_frame_index);

            if (current_memory_block->size < aligned_requested_size)
            {
                current_memory_block = current_memory_block->next;
                continue;
            }
            
            if (has_free_frames)
            {
                configure_memory_frames
                (
                    &(current_memory_block->memory_frames[start_frame_index]),
                    requested_page_count,
                    owner,
                    true
                );
                uint64_t start_frame_address = current_memory_block->physical_address + (start_frame_index * PAGE_SIZE);
                return reinterpret_cast<void*>(start_frame_address);
            }

            current_memory_block = current_memory_block->next;
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
                kernel::utility::logger::printk("current_memory_block->size >= size : %16llu >=%16llu\n", current_memory_block->size, size);
                kernel::utility::logger::printk("current_memory_block_address: %016p\n", current_memory_block);
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
            kernel::utility::logger::printk("i: %016llu | free_page_count: %016llu\e[0G", i, free_page_count);

            if (free_page_count == page_count)
            {
                start_frame_index = i + 1 - free_page_count;
                return true;
            }
        }
        kernel::utility::logger::split();
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
