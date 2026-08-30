#include <hal/interface/memory_manager.hpp>

#include <hal/aarch64/arch/cpu.hpp>
#include <hal/aarch64/memory/paging.hpp>
#include <kernel/memory/memory.hpp>
#include <liba9n/libc/string.hpp>

namespace a9n::hal
{
    namespace
    {
        using aarch64::descriptor_address_mask;

        a9n::word *table_pointer(a9n::physical_address address)
        {
            return a9n::kernel::physical_to_virtual_pointer<a9n::word>(address);
        }

        void invalidate_if_current(const a9n::kernel::page_table &root, a9n::virtual_address)
        {
            if (aarch64::current_user_page_table() == root.address)
            {
                aarch64::invalidate_tlb_all();
            }
        }

        kernel::memory_map_result<a9n::word *> traverse_page_table_entry(
            const a9n::kernel::page_table &root,
            a9n::virtual_address           address,
            a9n::word                      depth
        )
        {
            auto validation = validate_root_address_space(root);
            if (!validation)
            {
                return validation.unwrap_error();
            }
            if (depth < aarch64::page_depth::L3 || depth > aarch64::page_depth::L0)
            {
                return kernel::memory_map_error::ILLEGAL_DEPTH;
            }

            auto *table = table_pointer(root.address);
            for (a9n::word current_depth = aarch64::page_depth::L0; current_depth > depth;
                 --current_depth)
            {
                auto &entry = table[aarch64::calculate_page_table_index(address, current_depth)];
                if ((entry & aarch64::descriptor_valid) == 0
                    || (entry & aarch64::descriptor_table_page) == 0)
                {
                    return kernel::memory_map_error::NO_SUCH_PAGE_TABLE;
                }

                const auto next_address = entry & descriptor_address_mask;
                if (!next_address)
                {
                    return kernel::memory_map_error::NO_SUCH_PAGE_TABLE;
                }
                table = table_pointer(next_address);
            }

            return &table[aarch64::calculate_page_table_index(address, depth)];
        }
    }

    liba9n::result<a9n::kernel::page_table, hal_error>
        make_address_space(a9n::physical_address root_address)
    {
        if (!root_address || (root_address & (a9n::PAGE_SIZE - 1)) != 0)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        liba9n::std::memset(table_pointer(root_address), 0, a9n::PAGE_SIZE);
        return a9n::kernel::page_table { root_address, aarch64::page_depth::L0 };
    }

    a9n::word read_address_space_owners(const a9n::kernel::page_table &page_table)
    {
        return table_pointer(page_table.address)[aarch64::ADDRESS_SPACE_OWNERS_INDEX];
    }

    void write_address_space_owners(const a9n::kernel::page_table &page_table, a9n::word owners)
    {
        table_pointer(page_table.address)[aarch64::ADDRESS_SPACE_OWNERS_INDEX] = owners;
    }

    kernel::memory_map_result<> validate_root_address_space(const a9n::kernel::page_table &target_root)
    {
        if (target_root.get_depth() != aarch64::page_depth::L0)
        {
            return kernel::memory_map_error::ILLEGAL_DEPTH;
        }
        if (!target_root.address || (target_root.address & (a9n::PAGE_SIZE - 1)) != 0)
        {
            return kernel::memory_map_error::INVALID_PAGE_TABLE;
        }
        return {};
    }

    kernel::memory_map_result<> map_page_table(
        const a9n::kernel::page_table &target_root,
        const a9n::kernel::page_table &target_table,
        a9n::virtual_address           target_address,
        a9n::word
    )
    {
        if (!target_table.address || (target_table.address & (a9n::PAGE_SIZE - 1)) != 0)
        {
            return kernel::memory_map_error::INVALID_PAGE_TABLE;
        }
        if (target_table.get_depth() < aarch64::page_depth::L3
            || target_table.get_depth() >= target_root.get_depth())
        {
            return kernel::memory_map_error::ILLEGAL_DEPTH;
        }

        auto entry_result
            = traverse_page_table_entry(target_root, target_address, target_table.get_depth() + 1);
        if (!entry_result)
        {
            return entry_result.unwrap_error();
        }

        auto *entry = entry_result.unwrap();
        if (*entry & aarch64::descriptor_valid)
        {
            return kernel::memory_map_error::ALREADY_MAPPED;
        }
        *entry = (target_table.address & descriptor_address_mask) | aarch64::descriptor_valid
               | aarch64::descriptor_table_page;
        invalidate_if_current(target_root, target_address);
        return {};
    }

    kernel::memory_map_result<> unmap_page_table(
        const a9n::kernel::page_table &target_root,
        const a9n::kernel::page_table &target_table,
        a9n::virtual_address           target_address
    )
    {
        if (!target_table.address)
        {
            return kernel::memory_map_error::INVALID_PAGE_TABLE;
        }
        auto entry_result
            = traverse_page_table_entry(target_root, target_address, target_table.get_depth() + 1);
        if (!entry_result)
        {
            return entry_result.unwrap_error();
        }
        *entry_result.unwrap() = 0;
        invalidate_if_current(target_root, target_address);
        return {};
    }

    kernel::memory_map_result<> map_frame(
        const a9n::kernel::page_table &target_root,
        const a9n::kernel::frame      &target_frame,
        a9n::virtual_address           target_address,
        a9n::word                      rights
    )
    {
        if (!target_frame.address)
        {
            return kernel::memory_map_error::INVALID_FRAME;
        }

        auto depth_result = aarch64::convert_leaf_size_bits_to_internal_depth(target_frame.size_bits);
        if (!depth_result)
        {
            return kernel::memory_map_error::ILLEGAL_DEPTH;
        }

        const auto frame_size = static_cast<a9n::word>(1) << target_frame.size_bits;
        if ((target_frame.address & (frame_size - 1)) != 0 || (target_address & (frame_size - 1)) != 0)
        {
            return kernel::memory_map_error::ILLEGAL_DEPTH;
        }

        auto entry_result
            = traverse_page_table_entry(target_root, target_address, depth_result.unwrap());
        if (!entry_result)
        {
            return entry_result.unwrap_error();
        }
        auto *entry = entry_result.unwrap();
        if (*entry & aarch64::descriptor_valid)
        {
            return kernel::memory_map_error::ALREADY_MAPPED;
        }

        a9n::word descriptor
            = (target_frame.address & descriptor_address_mask) | aarch64::descriptor_valid
            | aarch64::descriptor_access | aarch64::descriptor_inner_shareable
            | aarch64::descriptor_attr_normal | aarch64::descriptor_user | aarch64::descriptor_pxn;
        if (depth_result.unwrap() == aarch64::page_depth::L3)
        {
            descriptor |= aarch64::descriptor_table_page;
        }
        if ((rights & static_cast<a9n::word>(kernel::rights::WRITE)) == 0)
        {
            descriptor |= aarch64::descriptor_read_only;
        }
        if ((rights & static_cast<a9n::word>(kernel::rights::EXECUTE)) == 0)
        {
            descriptor |= aarch64::descriptor_uxn;
        }

        *entry = descriptor;
        invalidate_if_current(target_root, target_address);
        return {};
    }

    kernel::memory_map_result<> unmap_frame(
        const a9n::kernel::page_table &target_root,
        const a9n::kernel::frame      &target_frame,
        a9n::virtual_address           target_address
    )
    {
        if (!target_frame.address)
        {
            return kernel::memory_map_error::INVALID_FRAME;
        }
        auto depth_result = aarch64::convert_leaf_size_bits_to_internal_depth(target_frame.size_bits);
        if (!depth_result)
        {
            return kernel::memory_map_error::ILLEGAL_DEPTH;
        }
        auto entry_result
            = traverse_page_table_entry(target_root, target_address, depth_result.unwrap());
        if (!entry_result)
        {
            return entry_result.unwrap_error();
        }
        *entry_result.unwrap() = 0;
        invalidate_if_current(target_root, target_address);
        return {};
    }

    kernel::memory_map_result<a9n::word> search_unset_page_table_depth(
        const a9n::kernel::page_table &target_root,
        a9n::virtual_address           target_address
    )
    {
        return search_unset_page_table_depth(target_root, target_address, 12);
    }

    kernel::memory_map_result<a9n::word> search_unset_page_table_depth(
        const a9n::kernel::page_table &target_root,
        a9n::virtual_address           target_address,
        a9n::word                      leaf_size_bits
    )
    {
        auto validation = validate_root_address_space(target_root);
        if (!validation)
        {
            return validation.unwrap_error();
        }
        auto depth_result = aarch64::convert_leaf_size_bits_to_internal_depth(leaf_size_bits);
        if (!depth_result)
        {
            return kernel::memory_map_error::ILLEGAL_DEPTH;
        }

        auto *table = table_pointer(target_root.address);
        for (a9n::word depth = aarch64::page_depth::L0; depth > depth_result.unwrap(); --depth)
        {
            auto entry = table[aarch64::calculate_page_table_index(target_address, depth)];
            if ((entry & aarch64::descriptor_valid) == 0)
            {
                return depth - 1;
            }
            if ((entry & aarch64::descriptor_table_page) == 0)
            {
                return kernel::memory_map_error::ALREADY_MAPPED;
            }
            table = table_pointer(entry & descriptor_address_mask);
        }
        return static_cast<a9n::word>(0);
    }

    kernel::memory_map_result<a9n::word>
        search_page_table_cover_size_by_depth(const a9n::kernel::page_table &target_root, a9n::word depth)
    {
        auto validation = validate_root_address_space(target_root);
        if (!validation)
        {
            return validation.unwrap_error();
        }
        switch (depth)
        {
            case aarch64::page_depth::L0 :
                return 512ULL * 512ULL * 512ULL * a9n::PAGE_SIZE;
            case aarch64::page_depth::L1 :
                return 512ULL * 512ULL * a9n::PAGE_SIZE;
            case aarch64::page_depth::L2 :
                return 512ULL * a9n::PAGE_SIZE;
            case aarch64::page_depth::L3 :
                return static_cast<a9n::word>(a9n::PAGE_SIZE);
            default :
                return kernel::memory_map_error::ILLEGAL_DEPTH;
        }
    }

    kernel::memory_map_result<> validate_frame_size_bits(a9n::word size_bits)
    {
        switch (size_bits)
        {
            case 12 :
            case 21 :
            case 30 :
                return {};
            default :
                return kernel::memory_map_error::ILLEGAL_DEPTH;
        }
    }
}
