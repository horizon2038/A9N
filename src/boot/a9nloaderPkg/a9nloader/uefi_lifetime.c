#include "uefi_lifetime.h"

EFI_STATUS
exit_uefi(EFI_HANDLE image_handle, uefi_memory_map *target_memory_map)
{
    /*
    char memory_map_buffer[4096 * 4];
    memory_map target_memory_map = {sizeof(memory_map_buffer),
    memory_map_buffer, 0, 0, 0, 0}; get_uefi_memory_map(&target_memory_map);
    */

    EFI_STATUS efi_status;
    efi_status = gBS->ExitBootServices(image_handle, target_memory_map->map_key);
    return efi_status;
}
