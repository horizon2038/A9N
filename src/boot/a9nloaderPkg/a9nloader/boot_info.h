#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include "stdint.h"

typedef struct
{
    uint64_t physical_memory_start;
    uint64_t page_count;
    bool is_free;
} memory_map_entry;

typedef struct
{
    uint64_t memory_size;
    memory_map_entry *memory_map;
    uint16_t memory_map_count;
} boot_info;

#endif
