#ifndef MEMORY_TYPE_H
#define MEMORY_TYPE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint64_t physical_address_start;
    uint64_t page_count;
    bool     is_free;
    bool     is_device;
} memory_map_entry;

typedef struct
{
    uint64_t          memory_size;
    uint16_t          memory_map_count;
    memory_map_entry *memory_map;
} memory_info;

#endif
