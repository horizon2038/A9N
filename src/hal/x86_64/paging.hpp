#ifndef X86_64_PAGING_HPP
#define X86_64_PAGING_HPP

#include <stdint.h>

namespace hal::x86_64
{
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
    };

    class page_table
    {
        public:
            page_table();
            ~page_table();

            uint64_t page_to_physical_address(uint16_t index);
            bool is_present(uint16_t index);

            page entries[PAGE_TABLE_COUNT] __attribute__((aligned(4096)));

        private:
            void init();
    };
}

#endif
