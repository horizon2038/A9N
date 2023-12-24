#ifndef X86_64_PAGING_HPP
#define X86_64_PAGING_HPP

#include "common.hpp"
#include <stdint.h>

#include <library/logger.hpp>

namespace hal::x86_64
{
    extern "C" uint64_t __kernel_pml4;
    extern "C" void _load_cr3(kernel::physical_address cr3_address);
    extern "C" void _flush_tlb();
    extern "C" void _invalidate_page(kernel::virtual_address target_virtual_address);

    constexpr static uint16_t PAGE_TABLE_COUNT = 512;

    union x86_64_virtual_address
    {
        uint64_t all;

        struct
        {
            uint64_t page : 12;
            uint64_t page_table : 9;
            uint64_t page_directory : 9;
            uint64_t page_directory_pointer : 9;
            uint64_t page_map_level_4 : 9;
            uint64_t canonical : 16;
        } __attribute__((packed));
    };

    union page
    {
        uint64_t all; 
        
        struct
        {
            uint64_t present : 1;
            uint64_t rw : 1;
            uint64_t user_supervisor : 1;
            uint64_t write_through : 1;
            uint64_t cache_disable : 1;
            uint64_t accessed : 1;
            uint64_t dirty : 1;
            uint64_t page_size : 1;
            uint64_t global : 1;
            uint64_t : 3;
            uint64_t address : 40;
            uint64_t : 12;
        } __attribute__((packed));

        kernel::physical_address get_physical_address()
        {
            return reinterpret_cast<kernel::physical_address>(address << 12);
        }

        void configure_physical_address(kernel::physical_address target_physical_address)
        {
            address = (target_physical_address >> 12);
        }
    };

    namespace PAGE_DEPTH
    {
        constexpr static uint16_t PML4 = 4;
        constexpr static uint16_t PDPT = 3;
        constexpr static uint16_t PD = 2;
        constexpr static uint16_t PT = 1;
        constexpr static uint16_t OFFSET = 0;
    }

    static inline uint64_t calculate_page_table_index(kernel::virtual_address target_virtual_address, uint16_t table_depth)
    {
        // depth = PAGE_DEPTH::{PAGE_TABLE_NAME}
        uint64_t shift = (table_depth > 0) ? (12 + (9 * (table_depth - 1))) : 0;
        return (target_virtual_address >> shift) & 0x1FF;
    }

}

#endif
