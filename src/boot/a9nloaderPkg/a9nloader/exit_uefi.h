#ifndef EXIT_UEFI_H
#define EXIT_UEFI_H

#include "memory_map.h"

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS get_memory_map(memory_map *target_memory_map)
{
    /*
    if (target_memory_map->buffer == nr)
    {
        return 0;
    }
    */
    
    target_memory_map->map_size = target_memory_map->buffer_size;
    return gBS->GetMemoryMap(&target_memory_map->map_size, target_memory_map->buffer, &target_memory_map->map_key, &target_memory_map->descriptor_size, &target_memory_map->descriptor_version);
}

EFI_STATUS exit_uefi(EFI_HANDLE image_handle)
{
    char memory_map_buffer[4096 * 4];
    memory_map target_memory_map = {sizeof(memory_map_buffer), memory_map_buffer, 0, 0, 0, 0}; 
    get_memory_map(&target_memory_map);
    
    EFI_STATUS efi_status;
    efi_status = gBS->ExitBootServices(image_handle, target_memory_map.map_size);
    return efi_status;
}

#endif
