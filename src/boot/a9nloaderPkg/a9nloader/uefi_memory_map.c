#include "uefi_memory_map.h"

EFI_STATUS get_uefi_memory_map(uefi_memory_map *target_memory_map)
{
    target_memory_map->map_size = target_memory_map->buffer_size;
    return gBS->GetMemoryMap(
        &target_memory_map->map_size,
        target_memory_map->buffer,
        &target_memory_map->map_key,
        &target_memory_map->descriptor_size,
        &target_memory_map->descriptor_version
    );
}
