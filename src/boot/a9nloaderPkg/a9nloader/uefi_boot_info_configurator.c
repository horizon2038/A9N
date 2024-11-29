#include "uefi_boot_info_configurator.h"

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Uefi.h>

#include "acpi.h"
#include "boot_info.h"
#include "memory_type.h"
#include "stdint.h"
#include "uefi_memory_map.h"

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

    if (target_uefi_memory_map->descriptor_size == 0)
    {
        return EFI_INVALID_PARAMETER;
    }

    uint16_t entries_count = target_uefi_memory_map->map_size / target_uefi_memory_map->descriptor_size;
    target_boot_info->boot_memory_info.memory_map_count = entries_count;
    target_boot_info->boot_memory_info.memory_size      = 0;

    EFI_PHYSICAL_ADDRESS memory_info_map_address        = 0;
    EFI_STATUS           efi_status                     = gBS->AllocatePages(
        AllocateAnyPages,
        EfiReservedMemoryType,
        EFI_SIZE_TO_PAGES(entries_count * sizeof(memory_map_entry)),
        &memory_info_map_address
    );
    target_boot_info->boot_memory_info.memory_map = (memory_map_entry *)memory_info_map_address;

    if (EFI_ERROR(efi_status))
        return efi_status;

    if (target_boot_info->boot_memory_info.memory_map == NULL)
    {
        return EFI_OUT_OF_RESOURCES;
    }
    for (uint16_t i = 0; i < entries_count; i++)
    {
        memory_map_entry *target_memory_map_entry = &target_boot_info->boot_memory_info.memory_map[i];
        EFI_MEMORY_DESCRIPTOR *uefi_desc
            = (EFI_MEMORY_DESCRIPTOR *)((char *)target_uefi_memory_map->buffer
                                        + i * target_uefi_memory_map->descriptor_size);

        target_memory_map_entry->physical_address_start = uefi_desc->PhysicalStart;
        target_memory_map_entry->page_count             = uefi_desc->NumberOfPages;

        switch (uefi_desc->Type)
        {
            // free
            case EfiConventionalMemory :
            case EfiBootServicesCode :
            case EfiBootServicesData :
                target_memory_map_entry->type = FREE_MEMORY;
                break;

            // device
            case EfiMemoryMappedIO :
            case EfiMemoryMappedIOPortSpace :
            case EfiACPIReclaimMemory :
            case EfiACPIMemoryNVS :
                target_memory_map_entry->type = DEVICE_MEMORY;
                break;

            // reserved
            case EfiRuntimeServicesCode :
            case EfiRuntimeServicesData :
            case EfiReservedMemoryType :
            default :
                target_memory_map_entry->type = RESERVED_MEMORY;
        }

        target_boot_info->boot_memory_info.memory_size
            += (target_memory_map_entry->page_count * EFI_PAGE_SIZE);
    }

    return EFI_SUCCESS;
}
