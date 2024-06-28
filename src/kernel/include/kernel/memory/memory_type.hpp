#ifndef MEMORY_TYPE_H
#define MEMORY_TYPE_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/types.hpp>

namespace a9n::kernel
{
    struct memory_map_entry
    {
        a9n::physical_address start_physical_address;
        a9n::word             page_count;
        bool                  is_free;
        bool                  is_device;
    };

    struct memory_info
    {
        a9n::word         memory_size;
        uint16_t          memory_map_count;
        memory_map_entry *memory_map;
    };
}

#endif
