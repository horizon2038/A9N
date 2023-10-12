#ifndef MEMORY_TYPE_H
#define MEMORY_TYPE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint64_t physical_address_start;
    uint64_t page_count;
    bool is_free;
    bool is_device;
} memory_map_entry;

typedef struct
{
    uint64_t memory_size;
    memory_map_entry *memory_map;
    uint16_t memory_map_count;
} memory_info;

#endif
