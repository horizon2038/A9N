#include <hal/x86_64/memory/memory_manager.hpp>

#include <hal/x86_64/arch/arch_types.hpp>
#include <hal/x86_64/memory/paging.hpp>

#include <kernel/utility/logger.hpp>

#include <library/common/types.hpp>
#include <library/libc/string.hpp>

namespace hal::x86_64
{
    void memory_manager::init_memory()
    {
        // init kernel memory mapping
        common::physical_address *kernel_top_page_table
            = reinterpret_cast<common::physical_address *>(&__kernel_pml4);

        kernel_top_page_table[0] = 0; // reset id-map
        _invalidate_page(0);
    }

    void memory_manager::init_virtual_memory(
        common::physical_address top_page_table_address
    )
    {
        // copy kernel_page_table.
        // no meltdown vulnerability countermeasures are taken because the page
        // table is not split.
        common::virtual_address virtual_top_page_table_address
            = convert_physical_to_virtual_address(top_page_table_address);
        common::virtual_address virtual_kernel_top_page_address
            = convert_physical_to_virtual_address(
                reinterpret_cast<common::physical_address>(&__kernel_pml4)
            );
        std::memcpy(
            reinterpret_cast<void *>(virtual_top_page_table_address),
            reinterpret_cast<void *>(virtual_kernel_top_page_address),
            common::PAGE_SIZE
        );
    }

    bool memory_manager::is_table_exists(
        common::physical_address top_page_table_address,
        common::virtual_address target_virtual_address
    )
    {
        common::virtual_address *current_page_table
            = reinterpret_cast<common::virtual_address *>(
                convert_physical_to_virtual_address(top_page_table_address)
            );

        // search kernel
        if (!top_page_table_address)
        {
            common::physical_address kernel_page_table_address
                = reinterpret_cast<common::physical_address>(&__kernel_pml4);
            current_page_table = reinterpret_cast<common::virtual_address *>(
                convert_physical_to_virtual_address(kernel_page_table_address)
            );
        }

        page current_page_table_entry;

        for (uint16_t i = PAGE_DEPTH::PML4; i > PAGE_DEPTH::PT; i--)
        {
            uint64_t current_page_table_index
                = calculate_page_table_index(target_virtual_address, i);
            current_page_table_entry.all
                = current_page_table[current_page_table_index];

            if (!current_page_table_entry.present)
            {
                return false;
            }

            current_page_table = reinterpret_cast<common::virtual_address *>(
                convert_physical_to_virtual_address(
                    current_page_table_entry.get_physical_address()
                )
            );
        }
        // offset

        uint64_t PT_INDEX = calculate_page_table_index(
            target_virtual_address,
            PAGE_DEPTH::PT
        );
        current_page_table_entry.all = current_page_table[PT_INDEX];
        return current_page_table_entry.present;
        // return pt_entry != 0;
    }

    void memory_manager::configure_page_table(
        common::physical_address top_page_table_address,
        common::virtual_address target_virtual_address,
        common::physical_address page_table_address
    )
    {
        common::virtual_address *current_page_table
            = reinterpret_cast<common::virtual_address *>(
                convert_physical_to_virtual_address(top_page_table_address)
            );

        if (!top_page_table_address)
        {
            common::physical_address kernel_page_table_address
                = reinterpret_cast<common::physical_address>(&__kernel_pml4);
            current_page_table = reinterpret_cast<common::virtual_address *>(
                convert_physical_to_virtual_address(kernel_page_table_address)
            );
        }

        page current_page_table_entry;

        for (uint16_t i = PAGE_DEPTH::PML4; i >= PAGE_DEPTH::PT; i--)
        {
            uint64_t current_page_table_index
                = calculate_page_table_index(target_virtual_address, i);
            current_page_table_entry.all
                = current_page_table[current_page_table_index];

            if (current_page_table_entry.present)
            {
                current_page_table
                    = reinterpret_cast<common::virtual_address *>(
                        convert_physical_to_virtual_address(
                            current_page_table_entry.get_physical_address()
                        )
                    );
                continue;
            }

            current_page_table_entry.configure_physical_address(
                page_table_address
            );
            current_page_table_entry.present = true;
            current_page_table_entry.rw = true;
            current_page_table[current_page_table_index]
                = current_page_table_entry.all;
            break;
        }
    }

    void memory_manager::map_virtual_memory(
        common::physical_address top_page_table_address,
        common::virtual_address target_virtual_address,
        common::physical_address target_physical_address
    )
    {
        common::virtual_address *current_page_table
            = reinterpret_cast<common::virtual_address *>(
                convert_physical_to_virtual_address(top_page_table_address)
            );

        if (!top_page_table_address)
        {
            common::physical_address kernel_page_table_address
                = reinterpret_cast<common::physical_address>(&__kernel_pml4);
            current_page_table = reinterpret_cast<common::virtual_address *>(
                convert_physical_to_virtual_address(kernel_page_table_address)
            );
        }

        page current_page_table_entry;

        for (uint16_t i = PAGE_DEPTH::PML4; i > PAGE_DEPTH::PT; i--)
        {
            // present bit is not checked. it is checked by is_table_exists()
            uint16_t current_page_table_index
                = calculate_page_table_index(target_virtual_address, i);
            current_page_table_entry.all
                = current_page_table[current_page_table_index];
            current_page_table = reinterpret_cast<common::virtual_address *>(
                convert_physical_to_virtual_address(
                    current_page_table_entry.get_physical_address()
                )
            );
            // kernel::utility::logger::printk("hal::map_virtual_memory
            // page_table_depth : %d\n", i);
        }

        uint64_t pt_index = calculate_page_table_index(
            target_virtual_address,
            PAGE_DEPTH::PT
        );
        current_page_table_entry.all = current_page_table[pt_index];
        current_page_table_entry.configure_physical_address(
            target_physical_address
        );
        current_page_table_entry.present = true;
        current_page_table_entry.rw = true;
        current_page_table[pt_index] = current_page_table_entry.all;
        _invalidate_page(target_virtual_address);
    }

    void memory_manager::unmap_virtual_memory(
        common::physical_address top_page_table_address,
        common::virtual_address target_virtual_addresss
    )
    {
        return;
    }

    common::virtual_address memory_manager::convert_physical_to_virtual_address(
        const common::physical_address target_physical_address
    )
    {
        return hal::x86_64::convert_physical_to_virtual_address(
            target_physical_address
        );
    }

    common::physical_address
        memory_manager::convert_virtual_to_physical_address(
            const common::virtual_address target_virtual_address
        )
    {
        return hal::x86_64::convert_virtual_to_physical_address(
            target_virtual_address
        );
    }
}
