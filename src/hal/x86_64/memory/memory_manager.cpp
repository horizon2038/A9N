#include <hal/interface/memory_manager.hpp>

#include <kernel/capability/page_table_capability.hpp>
#include <kernel/memory/memory.hpp>
#include <kernel/memory/memory_type.hpp>
#include <kernel/process/cpu.hpp>
#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

#include <hal/interface/cpu.hpp>
#include <hal/x86_64/arch/arch_types.hpp>
#include <hal/x86_64/memory/paging.hpp>

#include <liba9n/libc/string.hpp>

namespace a9n::hal
{
    // helper
    namespace
    {
        kernel::memory_map_result<> try_maybe_invalidate_page(
            const a9n::kernel::page_table &target_address_space,
            a9n::virtual_address           target_address
        )
        {
            DEBUG_LOG("try_maybe_invalidate_page");
            return a9n::hal::current_local_variable()
                .and_then(
                    [&](kernel::cpu_local_variable *variable) -> hal_result
                    {
                        // TLB flush for CR3 is postponed until process is loaded
                        if (!variable->current_process)
                        {
                            DEBUG_LOG("no process");
                            return {};
                        }

                        if (variable->current_process->root_address_space.type
                            != a9n::kernel::capability_type::ADDRESS_SPACE)
                        {
                            DEBUG_LOG("no address space");
                            return hal_error::ILLEGAL_ARGUMENT;
                        }

                        auto owner_address_space = a9n::kernel::convert_slot_data_to_page_table(
                            variable->current_process->root_address_space.data
                        );

                        if (target_address_space.address == owner_address_space.address)
                        {
                            DEBUG_LOG("invalidate page");
                            x86_64::_invalidate_page(target_address);
                        }

                        return {};
                    }
                )
                .transform_error(
                    [&](hal_error e) -> kernel::memory_map_error
                    {
                        return kernel::memory_map_error::ILLEGAL_AUTORITY;
                    }
                );
        }
    }

    static kernel::memory_map_result<hal::x86_64::page *> traverse_page_table_entry(
        const a9n::kernel::page_table &target_root,
        a9n::virtual_address           target_address,
        const a9n::word                depth
    );

    liba9n::result<a9n::kernel::page_table, hal_error>
        make_address_space(a9n::physical_address root_address)
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
                        a9n::kernel::utility::logger::printh("already mapped!");
                        return kernel::memory_map_error::ALREADY_MAPPED;
                    }

                    entry->configure_physical_address(target_table.address);
                    entry->present         = true;
                    entry->rw              = true;
                    entry->user_supervisor = true;

                    DEBUG_LOG("entry addr : 0x%016llx", reinterpret_cast<uint64_t>(entry));
                    DEBUG_LOG("entry.all : 0x%016llx", entry->all);

                    return try_maybe_invalidate_page(target_root, target_address);
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

                    return try_maybe_invalidate_page(target_root, target_address);
                }
            );
    }

    kernel::memory_map_result<> map_frame(
        const a9n::kernel::page_table &target_root,
        const a9n::kernel::frame      &target_frame,
        const a9n::virtual_address     target_address
    )
    {
        DEBUG_LOG("map_frame");
        if (!target_frame.address)
        {
            DEBUG_LOG("invalid frame");
            return kernel::memory_map_error::INVALID_FRAME;
        }

        return traverse_page_table_entry(target_root, target_address, x86_64::PAGE_DEPTH::PT)
            .and_then(
                [=](x86_64::page *entry) -> kernel::memory_map_result<>
                {
                    DEBUG_LOG("entry : 0x%016llx", reinterpret_cast<uint64_t>(entry));
                    if (entry->present)
                    {
                        a9n::kernel::utility::logger::printh("already mapped!\n");
                        return kernel::memory_map_error::ALREADY_MAPPED;
                    }

                    DEBUG_LOG("target_frame : 0x%016llx", target_frame.address);

                    entry->configure_physical_address(target_frame.address);
                    entry->present         = true;
                    entry->rw              = true;
                    entry->user_supervisor = true;

                    DEBUG_LOG("entry addr : 0x%016llx", reinterpret_cast<uint64_t>(entry));
                    DEBUG_LOG("entry.all : 0x%016llx", entry->all);

                    DEBUG_LOG("invalidate page");
                    return try_maybe_invalidate_page(target_root, target_address);
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

                    return try_maybe_invalidate_page(target_root, target_address);
                }
            );
    }

    kernel::memory_map_result<a9n::word> search_unset_page_table_depth(
        const a9n::kernel::page_table &target_root,
        const a9n::virtual_address     target_address
    )
    {
        return validate_root_address_space(target_root)
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
        return validate_root_address_space(target_root)
            .and_then(
                [=](void) -> kernel::memory_map_result<x86_64::page *>
                {
                    DEBUG_LOG(
                        "depth : %llu, from : 0x%016llx, to : 0x%016llx",
                        depth,
                        target_root.address,
                        target_address
                    );

                    if (depth > x86_64::PAGE_DEPTH::PML4 || depth < x86_64::PAGE_DEPTH::PT)
                        [[unlikely]]
                    {
                        a9n::kernel::utility::logger::printh("illegal target depth!\n");
                        return kernel::memory_map_error::ILLEGAL_DEPTH;
                    }

                    if (!target_root.address) [[unlikely]]
                    {
                        return kernel::memory_map_error::INVALID_PAGE_TABLE;
                    }

                    x86_64::page *current_table
                        = a9n::kernel::physical_to_virtual_pointer<x86_64::page>(target_root.address);

                    x86_64::page *current_entry {};

                    for (auto i = x86_64::PAGE_DEPTH::PML4; i > depth; i--)
                    {
                        DEBUG_LOG("current depth : %llu", i);
                        auto current_table_index
                            = x86_64::calculate_page_table_index(target_address, i);
                        DEBUG_LOG("current index : 0x%16llx", current_table_index);
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
                    DEBUG_LOG("index : 0x%16llx", current_table_index);
                    DEBUG_LOG(
                        "entry : 0x%16llx",
                        reinterpret_cast<uint64_t>(&current_table[current_table_index])
                    );

                    return current_entry;
                }
            );
    }

    kernel::memory_map_result<> validate_root_address_space(const a9n::kernel::page_table &target_root)
    {
        if (target_root.depth != x86_64::PAGE_DEPTH::PML4) [[unlikely]]
        {
            a9n::kernel::utility::logger::printh("illegal root depth!\n");
            return kernel::memory_map_error::ILLEGAL_DEPTH;
        }

        if (!target_root.address) [[unlikely]]
        {
            a9n::kernel::utility::logger::printh("invalid page table!\n");
            return kernel::memory_map_error::INVALID_PAGE_TABLE;
        }

        return {};
    }
}
