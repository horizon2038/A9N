#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include <stddef.h>
#include <stdint.h>

#include <liba9n/common/types.hpp>
#include <kernel/memory/memory_type.hpp>

#include <kernel/process/process.hpp>
#include <hal/interface/memory_manager.hpp>

namespace a9n::kernel
{
    // constexpr static uint16_t PAGE_SIZE = 4096;

    struct memory_frame
    {
        process *owner;
        bool     is_allocated;
    };

    struct memory_block
    {
        a9n::physical_address start_physical_address;
        size_t                   size;
        memory_block            *next;
        a9n::word             memory_frame_count;
        memory_frame memory_frames[1]; // C++ uses an array of 1 elements to
                                       // emulate a FAM (Flexible Array Members).
    };

    // future: delegate physical_memory_allocation_system to
    // physical_memory_server (user-space). future: delegate
    // virtual_memory_allocation_system to virtual_memory_server (user-space).

    class memory_manager
    {
      public:
        memory_manager(
            a9n::hal::memory_manager &target_memory_manager,
            const memory_info              &target_memory_info
        );

        // physical-memory management
        a9n::physical_address
             allocate_physical_memory(size_t size, process *owner);
        void deallocate_physical_memory(
            a9n::physical_address target_physical_address,
            size_t                   size
        );

        // virtual-memory management
        void init_virtual_memory(process *target_process);
        void map_virtual_memory(
            a9n::kernel::process         *target_process,
            a9n::virtual_address  target_virtual_address,
            a9n::physical_address target_physical_address,
            a9n::word             page_count
        );
        void unmap_virtual_memory(
            a9n::kernel::process        *target_process,
            a9n::virtual_address target_virtual_address,
            a9n::word            page_count
        );

        a9n::virtual_address convert_physical_to_virtual_address(
            a9n::physical_address target_physical_address
        );
        a9n::physical_address convert_virtual_to_physical_address(
            a9n::virtual_address target_virtual_address
        );

        void info_physical_memory();

      private:
        memory_block                   *head_memory_block;
        a9n::hal::memory_manager &_memory_manager;

        void         init(const memory_info &target_memory_info);
        void         init_memory_block(const memory_info &target_memory_info);
        void         print_memory_block_info(memory_block &target_memory_block);
        void         init_memory_frame(memory_block &target_memory_block);
        size_t       align_size(size_t size, uint16_t page_size);
        a9n::word align_physical_address(
            a9n::physical_address target_physical_address,
            uint16_t                 page_size
        );
        void configure_memory_frames(
            memory_frame *start_frame,
            a9n::word  end_frame_index,
            process      *owner,
            bool          flag
        );
        bool find_free_frames(
            memory_block &target_memory_block,
            a9n::word  page_count,
            a9n::word &start_frame_index
        );
        bool find_frames(
            memory_block &target_memory_block,
            a9n::word  page_count,
            a9n::word &start_frame_index,
            bool          flag
        );
    };
}

#endif
