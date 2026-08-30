#ifndef A9N_HAL_AARCH64_PAGING_HPP
#define A9N_HAL_AARCH64_PAGING_HPP

#include <hal/hal_result.hpp>
#include <kernel/types.hpp>

namespace a9n::hal::aarch64
{
    extern "C" a9n::word __kernel_l0;

    inline constexpr a9n::word PAGE_TABLE_ENTRY_COUNT     = 512;
    inline constexpr a9n::word ADDRESS_SPACE_OWNERS_INDEX = PAGE_TABLE_ENTRY_COUNT - 1;

    namespace page_depth
    {
        inline constexpr a9n::word L0 = 4;
        inline constexpr a9n::word L1 = 3;
        inline constexpr a9n::word L2 = 2;
        inline constexpr a9n::word L3 = 1;
    }

    inline constexpr a9n::word descriptor_valid           = 1ULL << 0;
    inline constexpr a9n::word descriptor_table_page      = 1ULL << 1;
    inline constexpr a9n::word descriptor_attr_normal     = 0ULL << 2;
    inline constexpr a9n::word descriptor_access          = 1ULL << 10;
    inline constexpr a9n::word descriptor_inner_shareable = 3ULL << 8;
    inline constexpr a9n::word descriptor_user            = 1ULL << 6;
    inline constexpr a9n::word descriptor_read_only       = 1ULL << 7;
    inline constexpr a9n::word descriptor_pxn             = 1ULL << 53;
    inline constexpr a9n::word descriptor_uxn             = 1ULL << 54;
    inline constexpr a9n::word descriptor_address_mask    = 0x0000FFFFFFFFF000ULL;

    inline constexpr a9n::word calculate_page_table_index(a9n::virtual_address address, a9n::word depth)
    {
        return (address >> (12 + 9 * (depth - 1))) & 0x1ff;
    }

    inline constexpr liba9n::result<a9n::word, hal_error>
        convert_leaf_size_bits_to_internal_depth(a9n::word size_bits)
    {
        switch (size_bits)
        {
            case 12 :
                return static_cast<a9n::word>(page_depth::L3);
            case 21 :
                return static_cast<a9n::word>(page_depth::L2);
            case 30 :
                return static_cast<a9n::word>(page_depth::L1);
            default :
                return hal_error::ILLEGAL_ARGUMENT;
        }
    }

    inline a9n::physical_address current_user_page_table()
    {
        a9n::word ttbr0 {};
        asm volatile("mrs %0, ttbr0_el1" : "=r"(ttbr0));
        return ttbr0 & descriptor_address_mask;
    }

    inline void load_user_page_table(a9n::physical_address address)
    {
        asm volatile("dsb ishst; msr ttbr0_el1, %0; isb; tlbi vmalle1is; dsb ish; isb"
                     :
                     : "r"(address)
                     : "memory");
    }
}

#endif
