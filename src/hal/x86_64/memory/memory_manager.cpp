#include "hal/interface/memory_manager.hpp"
#include "kernel/memory/memory.hpp"
#include "kernel/memory/memory_type.hpp"
#include <hal/x86_64/memory/memory_manager.hpp>

#include <hal/x86_64/arch/arch_types.hpp>
#include <hal/x86_64/memory/paging.hpp>

#include <kernel/utility/logger.hpp>

#include <kernel/types.hpp>
#include <liba9n/libc/string.hpp>

namespace a9n::hal::x86_64
{
    hal_result unmap_lower_memory_mapping(void)
    {
        a9n::physical_address *kernel_top_page_table
            = reinterpret_cast<a9n::physical_address *>(&__kernel_pml4);
        if (!kernel_top_page_table)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }

        kernel_top_page_table[0] = 0; // reset id-map
        _invalidate_page(0);

        return {};
    }

    void memory_manager::init_memory()
    {
        // init kernel memory mapping
        a9n::physical_address *kernel_top_page_table
            = reinterpret_cast<a9n::physical_address *>(&__kernel_pml4);

        kernel_top_page_table[0] = 0; // reset id-map
        _invalidate_page(0);
    }

    void memory_manager::init_virtual_memory(a9n::physical_address top_page_table_address)
    {
        // copy kernel_page_table.
        // no meltdown vulnerability countermeasures are taken because the page
        // table is not split.
        a9n::kernel::utility::logger::printk(
            "configure top page table "
            "address\n"
        );
        a9n::virtual_address virtual_top_page_table_address
            = convert_physical_to_virtual_address(top_page_table_address);
        a9n::virtual_address virtual_kernel_top_page_address = convert_physical_to_virtual_address(
            reinterpret_cast<a9n::physical_address>(&__kernel_pml4)
        );

        a9n::kernel::utility::logger::printk(
            "copy kernel top page table "
            "address\n"
        );
        liba9n::std::memcpy(
            reinterpret_cast<void *>(virtual_top_page_table_address),
            reinterpret_cast<void *>(virtual_kernel_top_page_address),
            a9n::PAGE_SIZE
        );
    }

    bool memory_manager::is_table_exists(
        a9n::physical_address top_page_table_address,
        a9n::virtual_address  target_virtual_address
    )
    {
        a9n::virtual_address *current_page_table = reinterpret_cast<a9n::virtual_address *>(
            convert_physical_to_virtual_address(top_page_table_address)
        );

        // search kernel
        if (!top_page_table_address)
        {
            a9n::physical_address kernel_page_table_address
                = reinterpret_cast<a9n::physical_address>(&__kernel_pml4);
            current_page_table = reinterpret_cast<a9n::virtual_address *>(
                convert_physical_to_virtual_address(kernel_page_table_address)
            );
        }

        page current_page_table_entry;

        for (uint16_t i = PAGE_DEPTH::PML4; i > PAGE_DEPTH::PT; i--)
        {
            uint64_t current_page_table_index = calculate_page_table_index(target_virtual_address, i);
            current_page_table_entry.all = current_page_table[current_page_table_index];

            if (!current_page_table_entry.present)
            {
                return false;
            }

            current_page_table = reinterpret_cast<a9n::virtual_address *>(
                convert_physical_to_virtual_address(current_page_table_entry.get_physical_address())
            );
        }
        // offset

        uint64_t PT_INDEX = calculate_page_table_index(target_virtual_address, PAGE_DEPTH::PT);
        current_page_table_entry.all = current_page_table[PT_INDEX];
        return current_page_table_entry.present;
        // return pt_entry != 0;
    }

    void memory_manager::configure_page_table(
        a9n::physical_address top_page_table_address,
        a9n::virtual_address  target_virtual_address,
        a9n::physical_address page_table_address
    )
    {
        a9n::virtual_address *current_page_table = reinterpret_cast<a9n::virtual_address *>(
            convert_physical_to_virtual_address(top_page_table_address)
        );

        if (!top_page_table_address)
        {
            a9n::physical_address kernel_page_table_address
                = reinterpret_cast<a9n::physical_address>(&__kernel_pml4);
            current_page_table = reinterpret_cast<a9n::virtual_address *>(
                convert_physical_to_virtual_address(kernel_page_table_address)
            );
        }

        page current_page_table_entry;

        for (uint16_t i = PAGE_DEPTH::PML4; i >= PAGE_DEPTH::PT; i--)
        {
            uint64_t current_page_table_index = calculate_page_table_index(target_virtual_address, i);
            current_page_table_entry.all = current_page_table[current_page_table_index];

            if (current_page_table_entry.present)
            {
                current_page_table = reinterpret_cast<a9n::virtual_address *>(
                    convert_physical_to_virtual_address(current_page_table_entry.get_physical_address())
                );
                continue;
            }

            current_page_table_entry.configure_physical_address(page_table_address);
            current_page_table_entry.present             = true;
            current_page_table_entry.rw                  = true;
            current_page_table[current_page_table_index] = current_page_table_entry.all;
            // TODO: test
            current_page_table_entry.user_supervisor = true;
            break;
        }
    }

    void memory_manager::map_virtual_memory(
        a9n::physical_address top_page_table_address,
        a9n::virtual_address  target_virtual_address,
        a9n::physical_address target_physical_address
    )
    {
        a9n::virtual_address *current_page_table = reinterpret_cast<a9n::virtual_address *>(
            convert_physical_to_virtual_address(top_page_table_address)
        );

        if (!top_page_table_address)
        {
            a9n::physical_address kernel_page_table_address
                = reinterpret_cast<a9n::physical_address>(&__kernel_pml4);
            current_page_table = reinterpret_cast<a9n::virtual_address *>(
                convert_physical_to_virtual_address(kernel_page_table_address)
            );
        }

        page current_page_table_entry;

        for (uint16_t i = PAGE_DEPTH::PML4; i > PAGE_DEPTH::PT; i--)
        {
            // present bit is not checked. it is checked by is_table_exists()
            uint16_t current_page_table_index = calculate_page_table_index(target_virtual_address, i);
            current_page_table_entry.all = current_page_table[current_page_table_index];
            current_page_table           = reinterpret_cast<a9n::virtual_address *>(
                convert_physical_to_virtual_address(current_page_table_entry.get_physical_address())
            );
            // a9n::kernel::utility::logger::printk("a9n::hal::map_virtual_memory
            // page_table_depth : %d\n", i);
        }

        uint64_t pt_index = calculate_page_table_index(target_virtual_address, PAGE_DEPTH::PT);
        current_page_table_entry.all = current_page_table[pt_index];
        current_page_table_entry.configure_physical_address(target_physical_address);
        current_page_table_entry.present = true;
        current_page_table_entry.rw      = true;
        current_page_table[pt_index]     = current_page_table_entry.all;
        // TODO: test
        current_page_table_entry.user_supervisor = true;
        _invalidate_page(target_virtual_address);
    }

    void memory_manager::unmap_virtual_memory(
        a9n::physical_address top_page_table_address,
        a9n::virtual_address  target_virtual_addresss
    )
    {
        return;
    }

    a9n::virtual_address memory_manager::convert_physical_to_virtual_address(
        const a9n::physical_address target_physical_address
    )
    {
        return a9n::hal::x86_64::convert_physical_to_virtual_address(target_physical_address);
    }

    a9n::physical_address memory_manager::convert_virtual_to_physical_address(
        const a9n::virtual_address target_virtual_address
    )
    {
        return a9n::hal::x86_64::convert_virtual_to_physical_address(target_virtual_address);
    }
}

namespace a9n::hal
{
    static kernel::memory_map_result<hal::x86_64::page *> traverse_page_table_entry(
        const a9n::kernel::page_table &target_root,
        a9n::virtual_address           target_address,
        const a9n::word                depth
    );

    static kernel::memory_map_result<>
        validate_root_page_table(const a9n::kernel::page_table &root_table);

    liba9n::result<a9n::kernel::page_table, hal_error>
        make_root_page_table(a9n::physical_address root_address)
    {
        // copy kernel_page_table.
        // no meltdown vulnerability countermeasures are taken because the page
        // table is not split.
        // => TODO: support PCID
        if (!root_address)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        a9n::virtual_address virtual_root = x86_64::convert_physical_to_virtual_address(root_address);
        a9n::virtual_address virtual_kernel_root = x86_64::convert_physical_to_virtual_address(
            reinterpret_cast<a9n::physical_address>(&x86_64::__kernel_pml4)
        );

        if (!virtual_root || !virtual_kernel_root)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        liba9n::std::memcpy(
            reinterpret_cast<void *>(virtual_root),
            reinterpret_cast<void *>(virtual_kernel_root),
            a9n::PAGE_SIZE
        );

        return a9n::kernel::page_table { .address = root_address, .depth = x86_64::PAGE_DEPTH::PML4 };
    }

    kernel::memory_map_result<> map_page_table(
        const a9n::kernel::page_table &target_root,
        const a9n::kernel::page_table &target_table,
        const a9n::virtual_address     target_address
    )
    {
        if (!target_table.address)
        {
            return kernel::memory_map_error::INVALID_PAGE_TABLE;
        }

        return traverse_page_table_entry(target_root, target_address, target_table.depth + 1)
            .and_then(
                [=](x86_64::page *entry) -> kernel::memory_map_result<>
                {
                    if (entry->present)
                    {
                        a9n::kernel::utility::logger::printh("already mapped!\n");
                        return kernel::memory_map_error::ALREADY_MAPPED;
                    }

                    entry->configure_physical_address(target_table.address);
                    entry->present         = true;
                    entry->rw              = true;
                    entry->user_supervisor = true;

                    a9n::kernel::utility::logger::printh(
                        "entry addr : 0x%016llx\n",
                        reinterpret_cast<uint64_t>(entry)
                    );
                    a9n::kernel::utility::logger::printh("entry.all : 0x%016llx\n", entry->all);
                    x86_64::_invalidate_page(target_address);

                    return {};
                }
            );
    }

    kernel::memory_map_result<> unmap_page_table(
        const a9n::kernel::page_table &target_root,
        const a9n::kernel::page_table &target_table,
        const a9n::virtual_address     target_address
    )
    {
        if (!target_table.address)
        {
            return kernel::memory_map_error::INVALID_PAGE_TABLE;
        }

        return traverse_page_table_entry(target_root, target_address, target_table.depth + 1)
            .and_then(
                [=](x86_64::page *entry) -> kernel::memory_map_result<>
                {
                    if (!entry->present)
                    {
                        // already unmapped
                        return {};
                    }

                    entry->init();
                    x86_64::_invalidate_page(target_address);

                    return {};
                }
            );
    }

    kernel::memory_map_result<> map_frame(
        const a9n::kernel::page_table &target_root,
        const a9n::kernel::frame      &target_frame,
        const a9n::virtual_address     target_address
    )
    {
        if (!target_frame.address)
        {
            return kernel::memory_map_error::INVALID_FRAME;
        }

        return traverse_page_table_entry(target_root, target_address, x86_64::PAGE_DEPTH::PT)
            .and_then(
                [=](x86_64::page *entry) -> kernel::memory_map_result<>
                {
                    if (entry->present)
                    {
                        a9n::kernel::utility::logger::printh("already mapped!\n");
                        return kernel::memory_map_error::ALREADY_MAPPED;
                    }

                    a9n::kernel::utility::logger::printh(
                        "target_frame : 0x%016llx\n",
                        target_frame.address
                    );

                    entry->configure_physical_address(target_frame.address);
                    entry->present         = true;
                    entry->rw              = true;
                    entry->user_supervisor = true;

                    a9n::kernel::utility::logger::printh(
                        "entry addr : 0x%016llx\n",
                        reinterpret_cast<uint64_t>(entry)
                    );
                    a9n::kernel::utility::logger::printh("entry.all : 0x%016llx\n", entry->all);

                    x86_64::_invalidate_page(target_address);

                    return {};
                }
            );
    }

    kernel::memory_map_result<> unmap_frame(
        const a9n::kernel::page_table &target_root,
        const a9n::kernel::frame      &target_frame,
        const a9n::virtual_address     target_address
    )
    {
        if (!target_frame.address)
        {
            return kernel::memory_map_error::INVALID_FRAME;
        }

        return traverse_page_table_entry(target_root, target_address, x86_64::PAGE_DEPTH::OFFSET)
            .and_then(
                [=](x86_64::page *entry) -> kernel::memory_map_result<>
                {
                    if (entry->present)
                    {
                        return kernel::memory_map_error::ALREADY_MAPPED;
                    }

                    entry->init();
                    x86_64::_invalidate_page(target_address);

                    return {};
                }
            );
    }

    kernel::memory_map_result<a9n::word> search_unset_page_table_depth(
        const a9n::kernel::page_table &target_root,
        const a9n::virtual_address     target_address
    )
    {
        return validate_root_page_table(target_root)
            .and_then(
                [=](void) -> kernel::memory_map_result<a9n::word>
                {
                    x86_64::page *virtual_root
                        = a9n::kernel::physical_to_virtual_pointer<x86_64::page>(target_root.address);
                    x86_64::page *current_table = virtual_root;
                    x86_64::page *current_entry {};

                    for (auto i = x86_64::PAGE_DEPTH::PML4; i > (x86_64::PAGE_DEPTH::OFFSET + 1); i--)
                    {
                        auto current_table_index
                            = x86_64::calculate_page_table_index(target_address, i + 1);
                        current_entry = &current_table[current_table_index];
                        if (!current_entry->present)
                        {
                            return i - 1;
                        }

                        auto next_physical_address = current_entry->get_physical_address();
                        if (!next_physical_address)
                        {
                            return kernel::memory_map_error::NO_SUCH_PAGE_TABLE;
                        }

                        current_table = a9n::kernel::physical_to_virtual_pointer<x86_64::page>(
                            next_physical_address
                        );
                    }

                    return 0;
                }

            );
    }

    static kernel::memory_map_result<hal::x86_64::page *> traverse_page_table_entry(
        const a9n::kernel::page_table &target_root,
        a9n::virtual_address           target_address,
        const a9n::word                depth
    )
    {
        return validate_root_page_table(target_root)
            .and_then(
                [=](void) -> kernel::memory_map_result<x86_64::page *>
                {
                    a9n::kernel::utility::logger::printh(
                        "depth : %llu, from : 0x%016llx, to : 0x%016llx\n",
                        depth,
                        target_root.address,
                        target_address
                    );

                    // is validation's responsibility right ???
                    // should it be here ???
                    [[unlikely]] if (depth > x86_64::PAGE_DEPTH::PML4 || depth < x86_64::PAGE_DEPTH::PT)
                    {
                        a9n::kernel::utility::logger::printh("illegal target depth!\n");
                        return kernel::memory_map_error::ILLEGAL_DEPTH;
                    }

                    [[unlikely]] if (!target_root.address)
                    {
                        return kernel::memory_map_error::INVALID_PAGE_TABLE;
                    }

                    x86_64::page *current_table
                        = a9n::kernel::physical_to_virtual_pointer<x86_64::page>(target_root.address);

                    x86_64::page *current_entry {};

                    for (auto i = x86_64::PAGE_DEPTH::PML4; i > depth; i--)
                    {
                        a9n::kernel::utility::logger::printh("current depth : %llu\n", i);
                        auto current_table_index
                            = x86_64::calculate_page_table_index(target_address, i);
                        a9n::kernel::utility::logger::printh(
                            "current index : 0x%16llx\n",
                            current_table_index
                        );
                        current_entry = &current_table[current_table_index];

                        if (!current_entry->present)
                        {
                            a9n::kernel::utility::logger::printh(
                                "no such next page table, no "
                                "present!\n"
                            );
                            a9n::kernel::utility::logger::printh(
                                "current entry : 0x%016llx\n",
                                current_entry->all
                            );
                            return kernel::memory_map_error::NO_SUCH_PAGE_TABLE;
                        }

                        auto next_physical_address = current_entry->get_physical_address();
                        if (!next_physical_address)
                        {
                            a9n::kernel::utility::logger::printh("no such next page table!\n");
                            return kernel::memory_map_error::NO_SUCH_PAGE_TABLE;
                        }

                        current_table = a9n::kernel::physical_to_virtual_pointer<x86_64::page>(
                            next_physical_address
                        );
                    }

                    auto current_table_index
                        = x86_64::calculate_page_table_index(target_address, depth);
                    current_entry = &current_table[current_table_index];

                    // log
                    a9n::kernel::utility::logger::printh("index : 0x%16llx\n", current_table_index);
                    a9n::kernel::utility::logger::printh(
                        "entry : 0x%16llx\n",
                        reinterpret_cast<uint64_t>(&current_table[current_table_index])
                    );

                    return current_entry;
                }
            );
    }

    static kernel::memory_map_result<>
        validate_root_page_table(const a9n::kernel::page_table &target_root)
    {
        [[unlikely]] if (target_root.depth != x86_64::PAGE_DEPTH::PML4)
        {
            a9n::kernel::utility::logger::printh("illegal root depth!\n");
            return kernel::memory_map_error::ILLEGAL_DEPTH;
        }

        if (!target_root.address)
        {
            a9n::kernel::utility::logger::printh("invalid page table!\n");
            return kernel::memory_map_error::INVALID_PAGE_TABLE;
        }

        return {};
    }
}
