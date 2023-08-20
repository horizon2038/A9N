#ifndef MEMORYMAP_H
#define MEMORYMAP_H

#include "stdint.h"

typedef struct
{
    uint64_t buffer_size;
    void* buffer;
    uint64_t map_size;
    uint64_t map_key;
    uint64_t descriptor_size;
    uint32_t descriptor_version;
} memory_map;

#endif
