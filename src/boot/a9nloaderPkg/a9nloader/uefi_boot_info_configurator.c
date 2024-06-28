#include "uefi_boot_info_configurator.h"

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "boot_info.h"
#include "uefi_memory_map.h"
#include "stdint.h"
#include "acpi.h"

EFI_STATUS make_boot_info(
    EFI_SYSTEM_TABLE *system_table,
    uefi_memory_map  *target_uefi_memory_map,
    boot_info        *target_boot_info
)
{
    for (uintmax_t i = 0; i < 8; i++)
    {
        target_boot_info->arch_info[i] = 0xdeadbeaf;
    }
    target_boot_info->arch_info[0] = (uintmax_t)(find_rsdp(system_table));

    uint16_t entries_count = target_uefi_memory_map->map_size
                           / target_uefi_memory_map->descriptor_size;
    target_boot_info->boot_memory_info.memory_map_count = entries_count;
    target_boot_info->boot_memory_info.memory_size      = 0;

    EFI_STATUS efi_status = gBS->AllocatePool(
        EfiLoaderData,
        entries_count * sizeof(memory_map_entry),
        (void **)&target_boot_info->boot_memory_info.memory_map
    );
    if (EFI_ERROR(efi_status))
        return efi_status;

    if (target_boot_info->boot_memory_info.memory_map == NULL)
    {
        return EFI_OUT_OF_RESOURCES;
    }
    for (uint16_t i = 0; i < entries_count; i++)
    {
        memory_map_entry *target_memory_map_entry
            = &target_boot_info->boot_memory_info.memory_map[i];
        EFI_MEMORY_DESCRIPTOR *uefi_desc
            = (EFI_MEMORY_DESCRIPTOR
                   *)((char *)target_uefi_memory_map->buffer
                      + i * target_uefi_memory_map->descriptor_size);

        target_memory_map_entry->physical_address_start
            = uefi_desc->PhysicalStart;
        target_memory_map_entry->page_count = uefi_desc->NumberOfPages;
        target_memory_map_entry->is_free
            = (uefi_desc->Type == EFI_CONVENTIONAL_MEMORY
               || uefi_desc->Type == EFI_BOOT_SERVICES_CODE
               || uefi_desc->Type == EFI_BOOT_SERVICES_DATA);
        target_boot_info->boot_memory_info.memory_map[i].is_device
            = (uefi_desc->Type == EFI_MEMORY_MAPPED_IO
               || uefi_desc->Type == EFI_MEMORY_MAPPED_IO_PORT_SPACE);
        target_boot_info->boot_memory_info.memory_size
            += (target_memory_map_entry->page_count * EFI_PAGE_SIZE);
    }

    return EFI_SUCCESS;
}
