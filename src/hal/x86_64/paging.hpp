#ifndef X86_64_PAGING_HPP
#define X86_64_PAGING_HPP

#include <stdint.h>

namespace hal::x86_64
{
    union page
    {
        uint64_t data; 
        
        struct entry
        {
            uint64_t present : 1;
            uint64_t rw : 1;
            uint64_t user_supervisor : 1;
            uint64_t page_write_through : 1;
            uint64_t page_cache_disable : 1;
            uint64_t accessed : 1;
            uint64_t : 1;
            uint64_t page_size : 1;
            uint64_t : 4;
            uint64_t address : 40;
            uint64_t : 12;
        } __attribute__((packed));
    };
}

#endif
