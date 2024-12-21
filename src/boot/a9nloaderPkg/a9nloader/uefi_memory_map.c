#include "uefi_memory_map.h"

static char memory_map_buffer[4096 * 8];

EFI_STATUS get_uefi_memory_map(uefi_memory_map *target_memory_map)
{
    target_memory_map->buffer      = (void *)memory_map_buffer;

    target_memory_map->map_size    = sizeof(memory_map_buffer);
    target_memory_map->buffer_size = sizeof(memory_map_buffer);

    return gBS->GetMemoryMap(
        &target_memory_map->map_size,
        target_memory_map->buffer,
        &target_memory_map->map_key,
        &target_memory_map->descriptor_size,
        &target_memory_map->descriptor_version
    );
}
