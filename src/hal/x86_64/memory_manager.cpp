#include "memory_manager.hpp"
#include "paging.hpp"
#include "common.hpp"

#include <library/string.hpp>

namespace hal::x86_64
{
    void init_memory()
    {
        // initialize memory_system;
    }

    void memory_manager::init_virtual_memory(kernel::process *target_process)
    {
        // init pml4
        std::memset(reinterpret_cast<void*>(target_process->page_table), 0, kernel::PAGE_SIZE);

        kernel::physical_address kernel_page_table_address = reinterpret_cast<kernel::physical_address>(&__kernel_pml4);

        // copy kernel_page_table.
        // no meltdown vulnerability countermeasures are taken because the page table is not split.
        std::memcpy(reinterpret_cast<void*>(target_process->page_table), reinterpret_cast<void*>(kernel_page_table_address), 4096);
    }

    void memory_manager::map_virtual_memory
    (
        kernel::process *target_process,
        kernel::virtual_address target_virtual_address,
        kernel::physical_address target_physical_address,
        uint64_t page_count
    )
    {
        kernel::physical_address top_page_table;

        if (!target_process)
        {
            top_page_table = reinterpret_cast<kernel::physical_address>(&__kernel_pml4);
        }

        top_page_table = reinterpret_cast<kernel::physical_address>(target_process->page_table);

        for (uint64_t i = 0; i < page_count; i++)
        {
            uint64_t address_offset = target_virtual_address + i * kernel::PAGE_SIZE;
            kernel::virtual_address page_virtual_address = target_virtual_address + i * kernel::PAGE_SIZE;
            kernel::physical_address page_physical_address = target_physical_address + i * kernel::PAGE_SIZE;
            // map_page(*top_page_table, page_virtual_address, page_physical_address);
        }
    }

    page_table *memory_manager::acquire_page_table(page_table &parent_page_table, uint64_t index)
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

    kernel::virtual_address memory_manager::convert_physical_to_virtual_address
    (
        const kernel::physical_address target_physical_address
    )
    {
        return hal::x86_64::convert_physical_to_virtual_address(target_physical_address);
    }

    kernel::physical_address memory_manager::convert_virtual_to_physical_address
    (
        const kernel::virtual_address target_virtual_address
    )
    {
        return hal::x86_64::convert_virtual_to_physical_address(target_virtual_address);
    }
}
