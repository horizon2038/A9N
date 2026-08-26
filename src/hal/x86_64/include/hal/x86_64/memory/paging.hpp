#ifndef X86_64_PAGING_HPP
#define X86_64_PAGING_HPP

#include <hal/x86_64/arch/arch_types.hpp>

#include <kernel/types.hpp>
#include <stdint.h>

#include <kernel/utility/logger.hpp>

namespace a9n::hal::x86_64
{
    extern "C" uint64_t __kernel_pml4;

    inline void _load_cr3(a9n::physical_address cr3_address)
    {
        asm volatile("mov %0, %%cr3" : : "r"(cr3_address) : "memory");
    }

    inline void _flush_tlb(void)
    {
        a9n::physical_address cr3_address;
        asm volatile("mov %%cr3, %0; mov %0, %%cr3" : "=&r"(cr3_address) : : "memory");
    }

    inline void _invalidate_page(a9n::virtual_address target_virtual_address)
    {
        asm volatile("invlpg (%0)" : : "r"(target_virtual_address) : "memory");
    }

    static constexpr uint16_t PAGE_TABLE_COUNT                = 512;
    static constexpr uint16_t ADDRESS_SPACE_OWNERS_PML4_INDEX = PAGE_TABLE_COUNT - 1;
    static constexpr uint16_t KERNEL_PML4_INDEX = (KERNEL_VIRTUAL_BASE >> 39) & (PAGE_TABLE_COUNT - 1);

    static_assert(ADDRESS_SPACE_OWNERS_PML4_INDEX != KERNEL_PML4_INDEX);

    // PML4[511] is outside A9N's lower-half user address space and the kernel's
    // direct-map entry. It is reserved for address-space-local HAL metadata.
    inline a9n::word read_address_space_owners(a9n::physical_address page_table_address)
    {
        auto *pml4 = convert_physical_to_virtual_pointer<a9n::word>(page_table_address);
        return pml4[ADDRESS_SPACE_OWNERS_PML4_INDEX];
    }

    inline void write_address_space_owners(a9n::physical_address page_table_address, a9n::word owners)
    {
        auto *pml4 = convert_physical_to_virtual_pointer<a9n::word>(page_table_address);
        pml4[ADDRESS_SPACE_OWNERS_PML4_INDEX] = owners;
    }

    union x86_64_virtual_address
    {
        uint64_t all;

        struct
        {
            uint64_t page                   : 12;
            uint64_t page_table             : 9;
            uint64_t page_directory         : 9;
            uint64_t page_directory_pointer : 9;
            uint64_t page_map_level_4       : 9;
            uint64_t canonical              : 16;
        } __attribute__((packed));
    };

    union page
    {
        uint64_t all;

        struct
        {
            uint64_t present         : 1;
            uint64_t rw              : 1;
            uint64_t user_supervisor : 1;
            uint64_t write_through   : 1;
            uint64_t cache_disable   : 1;
            uint64_t accessed        : 1;
            uint64_t dirty           : 1;
            uint64_t page_size       : 1;
            uint64_t global          : 1;
            uint64_t                 : 3;
            uint64_t address         : 40;
            uint64_t                 : 11;
            uint64_t execute_disable : 1;
        } __attribute__((packed));

        a9n::physical_address get_physical_address() const
        {
            return reinterpret_cast<a9n::physical_address>(address << 12);
        }

        void configure_physical_address(a9n::physical_address target_physical_address)
        {
            address = (target_physical_address >> 12);
        }

        void init()
        {
            all = 0;
        }
    };

    namespace PAGE_DEPTH
    {
        static constexpr uint16_t PML4   = 4;
        static constexpr uint16_t PDPT   = 3;
        static constexpr uint16_t PD     = 2;
        static constexpr uint16_t PT     = 1;
        static constexpr uint16_t OFFSET = 0;
    }

    inline bool SUPPORT_2MiB_PAGE = false;
    inline bool SUPPORT_1GiB_PAGE = false;

    inline constexpr uint64_t
        calculate_page_table_index(a9n::virtual_address target_virtual_address, uint16_t table_depth)
    {
        // depth = PAGE_DEPTH::{PAGE_TABLE_NAME}
        // uint64_t shift = (table_depth > 0) ? (12 + (9 * (table_depth - 1))) : 0;
        uint64_t shift = (table_depth > 0) ? (12 + (9 * (table_depth - 1))) : 12;
        // uint64_t shift = (table_depth > 0) ? (12 + (9 * (table_depth - 1))) : 12;
        return (target_virtual_address >> shift) & 0x1FF;
    }

    inline constexpr liba9n::result<uint16_t, a9n::hal::hal_error>
        convert_leaf_size_bits_to_internal_depth(a9n::word leaf_size_bits)
    {
        switch (leaf_size_bits)
        {
            // 4KiB Leaf
            case 12 :
                return static_cast<uint16_t>(PAGE_DEPTH::PT);
            // 2MiB Leaf
            case 21 :
                return static_cast<uint16_t>(PAGE_DEPTH::PD);
            // 1GiB Leaf
            case 30 :
                return static_cast<uint16_t>(PAGE_DEPTH::PDPT);
            default :
                return a9n::hal::hal_error::ILLEGAL_ARGUMENT;
        }
    }
}

#endif
