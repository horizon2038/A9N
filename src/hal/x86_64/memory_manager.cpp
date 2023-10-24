#include "memory_manager.hpp"
#include "paging.hpp"
#include "common.hpp"

#include <library/string.hpp>

namespace hal::x86_64
{
    void init_memory()
    {
        // init kernel memory mapping
        kernel::physical_address *kernel_top_page_table = reinterpret_cast<kernel::physical_address*>(&__kernel_pml4);
        kernel_top_page_table[0] = 0; // reset id-map
    }

    void memory_manager::init_virtual_memory(kernel::physical_address top_page_table_address)
    {
        // copy kernel_page_table.
        // no meltdown vulnerability countermeasures are taken because the page table is not split.
        std::memcpy(reinterpret_cast<void*>(top_page_table_address), reinterpret_cast<void*>(&__kernel_pml4), kernel::PAGE_SIZE);
    }

    bool memory_manager::is_table_exists
    (
        kernel::physical_address top_page_table_address,
        kernel::virtual_address target_virtual_address
    )
    {
        kernel::physical_address *current_page_table = reinterpret_cast<kernel::physical_address*>(top_page_table_address);
        page current_page_table_entry;

        for (uint16_t i = PAGE_DEPTH::PML4; i > PAGE_DEPTH::PT; i--)
        {
            uint64_t current_page_table_index = calculate_page_table_index(target_virtual_address, i);
            current_page_table_entry.all = current_page_table[current_page_table_index];

            if (!current_page_table_entry.present)
            {
                return false;
            }

            current_page_table = reinterpret_cast<kernel::physical_address*>(current_page_table_entry.get_physical_address());
        }

        uint64_t pt_index = calculate_page_table_index(target_virtual_address, PAGE_DEPTH::PT); 
        kernel::physical_address pt_entry = current_page_table[pt_entry];
        return pt_entry != 0;
    }

    void memory_manager::configure_page_table
    (
        kernel::physical_address top_page_table_address,
        kernel::virtual_address target_virtual_address,
        kernel::physical_address page_table_address
    )
    {
        kernel::physical_address *current_page_table = reinterpret_cast<kernel::physical_address*>(top_page_table_address);
        page current_page_table_entry;

        for (uint16_t i = PAGE_DEPTH::PML4; i > PAGE_DEPTH::PT; i--)
        {
            uint64_t current_page_table_index = calculate_page_table_index(target_virtual_address, i);
            current_page_table_entry.all = current_page_table[current_page_table_index];

            if (!current_page_table_entry.present)
            {
                current_page_table_entry.configure_physical_address(page_table_address);
                current_page_table_entry.present = true;
                current_page_table[current_page_table_index] = current_page_table_entry.all;
                return;
            }

            current_page_table = reinterpret_cast<kernel::physical_address*>(current_page_table_entry.get_physical_address());
        }

        uint64_t pt_index = calculate_page_table_index(target_virtual_address, PAGE_DEPTH::PT);
        current_page_table[pt_index] = page_table_address;
    }

    void memory_manager::map_virtual_memory
    (
        kernel::physical_address top_page_table_address,
        kernel::virtual_address target_virtual_address,
        kernel::physical_address target_physical_address
    )
    {
        kernel::physical_address *current_page_table = reinterpret_cast<kernel::physical_address*>(top_page_table_address);
        page current_page_table_entry;

        for (uint16_t i = PAGE_DEPTH::PML4; i > PAGE_DEPTH::PT; i--)
        {
            // present bit is not checked. it is checked by is_table_exists()
            uint16_t current_page_table_index = calculate_page_table_index(target_virtual_address, i);
            current_page_table_entry.all = current_page_table[current_page_table_index];
            current_page_table = reinterpret_cast<kernel::physical_address*>(current_page_table_entry.get_physical_address());
        }

        uint64_t pt_index = calculate_page_table_index(target_virtual_address, PAGE_DEPTH::PT);
        current_page_table[pt_index] = target_physical_address;
    }

    void memory_manager::unmap_virtual_memory
    (
        kernel::physical_address top_page_table_address,
        kernel::virtual_address target_virtual_addresss
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
