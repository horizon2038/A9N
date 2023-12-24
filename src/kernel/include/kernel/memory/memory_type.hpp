#ifndef MEMORY_TYPE_H
#define MEMORY_TYPE_H

#include <stdint.h>
#include <stdbool.h>
#include <library/common/types.hpp>

namespace kernel
{
    struct memory_map_entry
    {
        common::physical_address start_physical_address;
        common::word page_count;
        bool is_free;
        bool is_device;
    };

    struct memory_info
    {
        common::word memory_size;
        uint16_t memory_map_count;
        memory_map_entry *memory_map;
    };
}

#endif
