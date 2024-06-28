#include <kernel/memory/memory_manager.hpp>

#include <kernel/utility/logger.hpp>
#include <library/libc/string.hpp>
#include <library/common/types.hpp>

namespace kernel
{
    memory_manager::memory_manager(
        hal::interface::memory_manager &target_memory_manager,
        const memory_info              &target_memory_info
    )
        : _memory_manager(target_memory_manager)
        , head_memory_block(nullptr)
    {
        init(target_memory_info);
    }

    void memory_manager::init(const memory_info &target_memory_info)
    {
        kernel::utility::logger::printk("init physical_memory\n");
        init_memory_block(target_memory_info);

        kernel::utility::logger::printk("init virtual_memory\n");
        _memory_manager.init_memory();
        // init kernel_page_map
    }

    void memory_manager::init_memory_block(const memory_info &target_memory_info
    )
    {
        /*
        memory_block is like a header placed at the beginning of free memory.
        therefore, memory allocations are not required
        (since it is assumed that free memory exists).
        */

        memory_map_entry *_memory_map_entry     = nullptr;
        memory_block     *previous_memory_block = nullptr;

        for (uint16_t i = 0; i < target_memory_info.memory_map_count; i++)
        {
            _memory_map_entry = &target_memory_info.memory_map[i];

            if (!(_memory_map_entry->is_free))
            {
                continue;
            }

            common::word memory_block_size
                = sizeof(memory_block)
                + sizeof(memory_frame) * (_memory_map_entry->page_count - 1);
            common::physical_address adjusted_address
                = _memory_map_entry->start_physical_address + memory_block_size;
            adjusted_address
                = align_physical_address(adjusted_address, common::PAGE_SIZE);
            size_t adjusted_size
                = common::PAGE_SIZE * (_memory_map_entry->page_count)
                - memory_block_size;

            memory_block *current_memory_block
                = reinterpret_cast<memory_block *>(
                    convert_physical_to_virtual_address(
                        _memory_map_entry->start_physical_address
                    )
                );
            // kernel::utility::logger::printk("current_memory_block_address :
            // 0x%llx\n",
            // reinterpret_cast<virtual_address>(current_memory_block));
            current_memory_block->start_physical_address = adjusted_address;
            current_memory_block->size                   = adjusted_size;
            current_memory_block->next                   = nullptr;
            current_memory_block->memory_frame_count
                = adjusted_size / common::PAGE_SIZE;

            init_memory_frame(*current_memory_block);

            if (head_memory_block == nullptr)
            {
                head_memory_block     = current_memory_block;
                previous_memory_block = current_memory_block;
                continue;
            }

            previous_memory_block->next = current_memory_block;
            previous_memory_block       = current_memory_block;
        }

        memory_block *now_memory_block = head_memory_block;

        /*
        while (now_memory_block)
        {
            print_memory_block_info(*now_memory_block);
            now_memory_block = now_memory_block->next;
        }
        */
    }

    void memory_manager::print_memory_block_info(
        memory_block &target_memory_block
    )
    {
        using logger = kernel::utility::logger;

        common::word memory_map_size
            = target_memory_block.memory_frame_count * common::PAGE_SIZE;
        common::word memory_map_size_kb = memory_map_size / 1024;
        common::word memory_map_size_mb = memory_map_size_kb / 1024;

        logger::printk("----- memory_block_info\e[60G%16s\n", "-----");
        logger::printk(
            "physical_address\e[52G:\e[58G0x%016llx\n",
            target_memory_block.start_physical_address
        );
        logger::printk(
            "next_physical_address\e[52G:\e[58G0x%016llx\n",
            (target_memory_block.next)->start_physical_address
        );
        logger::printk(
            "size\e[52G:\e[60G%16llu B | %6llu KiB | %6llu MiB \n",
            memory_map_size,
            memory_map_size_kb,
            memory_map_size_mb
        );
        // logger::printk("memory_frame_count\e[52G:\e[60G%16llu x 4KiB\n",
        // target_memory_block.memory_frame_count);
        logger::split();
    }

    void memory_manager::info_physical_memory()
    {
        memory_block *current_memory_block = head_memory_block;
        while (current_memory_block)
        {
            if (current_memory_block->next == nullptr)
            {
                return;
            }
            print_memory_block_info(*current_memory_block);
            current_memory_block = current_memory_block->next;
        }
    }

    void memory_manager::init_memory_frame(memory_block &target_memory_block)
    {
        for (common::word i = 0; i < target_memory_block.memory_frame_count;
             i++)
        {
            target_memory_block.memory_frames[i].owner        = nullptr;
            target_memory_block.memory_frames[i].is_allocated = false;
        }
    }

    common::physical_address
        memory_manager::allocate_physical_memory(size_t size, process *owner)
    {
        size_t aligned_requested_size = align_size(size, common::PAGE_SIZE);
        size_t requested_page_count
            = aligned_requested_size / common::PAGE_SIZE;
        memory_block *current_memory_block = head_memory_block;
        common::word  start_frame_index;
        bool          has_free_frames;

        while (current_memory_block)
        {
            has_free_frames = find_free_frames(
                *current_memory_block,
                requested_page_count,
                start_frame_index
            );

            if (current_memory_block->size < aligned_requested_size)
            {
                current_memory_block = current_memory_block->next;
                continue;
            }

            if (has_free_frames)
            {
                configure_memory_frames(
                    &(current_memory_block->memory_frames[start_frame_index]),
                    requested_page_count,
                    owner,
                    true
                );
                common::physical_address start_frame_address
                    = current_memory_block->start_physical_address
                    + (start_frame_index * common::PAGE_SIZE);
                kernel::utility::logger::printk(
                    "allocate_physical_memory : 0x%016llx , %llu B\n",
                    start_frame_address,
                    aligned_requested_size
                );
                return start_frame_address;
            }

            current_memory_block = current_memory_block->next;
        }

        return 0;
    }

    bool memory_manager::find_free_frames(
        memory_block &target_memory_block,
        common::word  page_count,
        common::word &start_frame_index
    )
    {
        return find_frames(
            target_memory_block,
            page_count,
            start_frame_index,
            false
        );
    }

    bool memory_manager::find_frames(
        memory_block &target_memory_block,
        common::word  page_count,
        common::word &start_frame_index,
        bool          flag
    )
    {
        common::word free_page_count = 0;

        for (common::word i = 0; i < target_memory_block.memory_frame_count;
             i++)
        {
            if (target_memory_block.memory_frames[i].is_allocated != flag)
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

    void memory_manager::configure_memory_frames(
        memory_frame *start_frame,
        common::word  page_count,
        process      *owner,
        bool          flag
    )
    {
        for (common::word i = 0; i < page_count; i++)
        {
            start_frame[i].is_allocated = flag;
            start_frame[i].owner        = owner;
        }
    }

    void memory_manager::deallocate_physical_memory(
        common::physical_address start_physical_address,
        size_t                   size
    )
    {
        memory_block *current_memory_block = head_memory_block;
        common::word  page_count
            = align_size(size, common::PAGE_SIZE) / common::PAGE_SIZE;
        common::word start_frame_index = 0;

        while (current_memory_block)
        {
            common::physical_address end_physical_address
                = start_physical_address + align_size(size, common::PAGE_SIZE);
            common::physical_address end_memory_block_address
                = current_memory_block->start_physical_address
                + current_memory_block->size;

            if (start_physical_address
                    > current_memory_block->start_physical_address
                || end_memory_block_address < end_physical_address)
            {
                current_memory_block = current_memory_block->next;
                continue;
            }

            start_frame_index = (start_physical_address
                                 - current_memory_block->start_physical_address)
                              / common::PAGE_SIZE;
            configure_memory_frames(
                &(current_memory_block->memory_frames[start_frame_index]),
                page_count,
                nullptr,
                false
            );
            break;
        }
    }

    size_t memory_manager::align_size(size_t size, uint16_t page_size)
    {
        size_t aligned_size = (size + page_size - 1) & ~(page_size - 1);
        return aligned_size;
    }

    common::word memory_manager::align_physical_address(
        common::word physical_address,
        uint16_t     page_size
    )
    {
        common::word aligned_address
            = (physical_address + page_size - 1) & ~(page_size - 1);
        return aligned_address;
    }

    common::virtual_address memory_manager::convert_physical_to_virtual_address(
        common::physical_address target_physical_address
    )
    {
        return _memory_manager.convert_physical_to_virtual_address(
            target_physical_address
        );
    }

    common::physical_address
        memory_manager::convert_virtual_to_physical_address(
            common::virtual_address target_virtual_address
        )
    {
        return _memory_manager.convert_virtual_to_physical_address(
            target_virtual_address
        );
    }

    void memory_manager::init_virtual_memory(process *target_process)
    {
        if (!target_process)
        {
            return; // error
        }

        // allocate top_page_table
        common::physical_address page_table_address
            = allocate_physical_memory(common::PAGE_SIZE, target_process);
        if (page_table_address == 0)
        {
            return; // error
        }

        target_process->page_table = page_table_address;
        library::std::memset(
            reinterpret_cast<void *>(
                convert_physical_to_virtual_address(target_process->page_table)
            ),
            0,
            common::PAGE_SIZE
        );

        _memory_manager.init_virtual_memory(target_process->page_table);

        return;
    }

    void memory_manager::map_virtual_memory(
        kernel::process         *target_process,
        common::virtual_address  target_virtual_address,
        common::physical_address target_physical_address,
        common::word             page_count
    )
    {
        bool                     is_kernel = !(target_process);
        common::physical_address target_page_table_address;
        target_page_table_address = target_process->page_table;

        if (is_kernel)
        {
            target_page_table_address = 0;
            utility::logger::printk("map_virtual_memory : map for kernel\n");
        }
        else
        {
            utility::logger::printk("map_virtual_memory : map for user\n");
        }

        for (common::word i = 0; i < page_count; i++)
        {
            common::word address_offset = i * common::PAGE_SIZE;

            while (true)
            {
                bool is_table_exists;
                is_table_exists = _memory_manager.is_table_exists(
                    target_page_table_address,
                    target_virtual_address + address_offset
                );

                if (is_table_exists)
                {
                    _memory_manager.map_virtual_memory(
                        target_page_table_address,
                        target_virtual_address + address_offset,
                        target_physical_address + address_offset
                    );
                    break;
                }

                common::physical_address allocated_page_table;
                allocated_page_table = allocate_physical_memory(
                    common::PAGE_SIZE,
                    is_kernel ? nullptr : target_process
                );
                _memory_manager.configure_page_table(
                    target_page_table_address,
                    target_virtual_address + address_offset,
                    allocated_page_table
                );
            }
        }
    }

    void memory_manager::unmap_virtual_memory(
        kernel::process         *target_process,
        common::virtual_address  target_virtual_address,
        common::physical_address target_physical_address
    )
    {
    }

}
