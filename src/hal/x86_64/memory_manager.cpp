#include "memory_manager.hpp"
#include "paging.hpp"

#include <library/string.hpp>

namespace hal::x86_64
{
    void init_memory()
    {
        // initialize memory_system;
    }

    void memory_manager::init_virtual_memory(uint64_t top_page_table_address)
    {
        // allocate page_table (pml4) ;
        std::memset(reinterpret_cast<void*>(top_page_table_address), 0, 4096);

        uint64_t kernel_page_table_address = 0x100000;

        std::memcpy(reinterpret_cast<void*>(top_page_table_address), reinterpret_cast<void*>(kernel_page_table_address), 4096);
    }

    void memory_manager::map_virtual_memory
    (
        kernel::process *target_process,
        uint64_t virtual_addresss,
        uint64_t physical_address,
        uint64_t page_count
    )
    {
        page_table *top_page_table;

        if (!target_process)
        {
            top_page_table = &kernel_top_page_table;
        }

        for (uint64_t i = 0; i < page_count; i++)
        {
            uint64_t page_virtual_address = virtual_addresss + i * PAGE_SIZE;
            uint64_t page_physical_address = physical_address + i * PAGE_SIZE;
            map_page(*top_page_table, page_virtual_address, page_physical_address);
        }
    }

    page_table *acquire_page_table(page_table &parent_page_table, uint64_t index)
    {
        if (parent_page_table.is_present(index))
        {
            return nullptr;
        }
        uint64_t physical_address = parent_page_table.page_to_physical_address(index);
        // Now it reutnrs a physical address dirctly, but this method will change to return a virtual address. 
        page_table* page_table_pointer = reinterpret_cast<page_table*>(physical_address); 
        return page_table_pointer;
    }

    void memory_manager::map_page(page_table &page_table, uint64_t virtual_address, uint64_t physical_address)
    {
        x86_64_virtual_address arch_virtual_address;
        arch_virtual_address.all = virtual_address;
    }

    void memory_manager::unmap_virtual_memory
    (
        kernel::process *target_process,
        uint64_t virtual_addresss,
        uint64_t page_count
    )
    {
    }
}
