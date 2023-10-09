#include "uefi_boot_info_configurator.h"

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "boot_info.h"
#include "uefi_memory_map.h"
#include "stdint.h"

EFI_STATUS make_boot_info(uefi_memory_map *target_uefi_memory_map, boot_info *target_boot_info)
{
    uint16_t entries_count = target_uefi_memory_map->map_size / target_uefi_memory_map->descriptor_size;
    target_boot_info->memory_map_count = entries_count;
    // Allocate memory for the boot_info memory_map
    // target_boot_info->memory_map = (memory_map_entry *) AllocatePool(entries_count * sizeof(memory_map_entry));
    EFI_STATUS efi_status = gBS->AllocatePool
    (
        EfiLoaderData,
        entries_count * sizeof(memory_map_entry),
        (void**)&target_boot_info->memory_map
    );
    if(EFI_ERROR(efi_status)) return efi_status;

    if (target_boot_info->memory_map == NULL)
    {
        return EFI_OUT_OF_RESOURCES;
    }
    for (uint16_t i = 0; i < entries_count; i++)
    {
        EFI_MEMORY_DESCRIPTOR *uefi_desc = (EFI_MEMORY_DESCRIPTOR*)
        (
            (char *)target_uefi_memory_map->buffer + i * target_uefi_memory_map->descriptor_size
        );

        target_boot_info->memory_map[i].physical_address_start = uefi_desc->PhysicalStart;
        target_boot_info->memory_map[i].page_count = uefi_desc->NumberOfPages;
        target_boot_info->memory_map[i].is_free =
        (
            uefi_desc->Type == EFI_CONVENTIONAL_MEMORY ||
            uefi_desc->Type == EFI_BOOT_SERVICES_CODE ||
            uefi_desc->Type == EFI_BOOT_SERVICES_DATA
        );
        target_boot_info->memory_map[i].is_device =
        (
            uefi_desc->Type == EFI_MEMORY_MAPPED_IO ||
            uefi_desc->Type == EFI_MEMORY_MAPPED_IO_PORT_SPACE
        );
    }
    // Calculate the total memory size based on the memory map
    target_boot_info->memory_size = 0;
    for (uint16_t i = 0; i < entries_count; i++)
    {
        target_boot_info->memory_size += (target_boot_info->memory_map[i].page_count * EFI_PAGE_SIZE);
    }

    return EFI_SUCCESS;
}
