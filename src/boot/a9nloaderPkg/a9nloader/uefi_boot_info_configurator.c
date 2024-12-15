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
    Print(L"make boot_info ...\r\n");
    if (!target_boot_info->boot_memory_info.memory_map)
    {
        return EFI_NOT_FOUND;
    }

    if (target_uefi_memory_map->descriptor_size == 0)
    {
        return EFI_INVALID_PARAMETER;
    }

    for (uintmax_t i = 0; i < 8; i++)
    {
        target_boot_info->arch_info[i] = 0xdeadbeaf;
    }
    target_boot_info->arch_info[0] = (uintmax_t)(find_rsdp(system_table));

    UINTN entries_count = target_uefi_memory_map->map_size / target_uefi_memory_map->descriptor_size;
    target_boot_info->boot_memory_info.memory_map_count = 0;
    target_boot_info->boot_memory_info.memory_size      = 0;
    memory_map_entry *last_entry                        = NULL;

    for (UINTN i = 0; i < entries_count; i++)
    {
        Print(L"entries count [%04llx] \r\n", i);
        // memory_map_entry *target_memory_map_entry =
        // &target_boot_info->boot_memory_info.memory_map[i];
        EFI_MEMORY_DESCRIPTOR *uefi_desc
            = (EFI_MEMORY_DESCRIPTOR *)((char *)target_uefi_memory_map->buffer
                                        + i * target_uefi_memory_map->descriptor_size);

        memory_map_entry new_entry;
        new_entry.physical_address_start = uefi_desc->PhysicalStart;
        new_entry.page_count             = uefi_desc->NumberOfPages;

        switch (uefi_desc->Type)
        {
            // free
            case EfiConventionalMemory :
            case EfiBootServicesCode :
            case EfiBootServicesData :
                new_entry.type = FREE_MEMORY;
                break;

            // device
            case EfiMemoryMappedIO :
            case EfiMemoryMappedIOPortSpace :
            case EfiACPIReclaimMemory :
            case EfiACPIMemoryNVS :
                new_entry.type = DEVICE_MEMORY;
                break;

            // reserved
            case EfiRuntimeServicesCode :
            case EfiRuntimeServicesData :
            case EfiReservedMemoryType :
            default :
                new_entry.type = RESERVED_MEMORY;
        }

        // combine memory areas
        if (last_entry
            && (last_entry->physical_address_start + (last_entry->page_count * EFI_PAGE_SIZE)
                == new_entry.physical_address_start)
            && (last_entry->type == new_entry.type))
        {
            last_entry->page_count += new_entry.page_count;
        }
        else
        {
            memory_map_entry *current_entry
                = &target_boot_info->boot_memory_info
                       .memory_map[target_boot_info->boot_memory_info.memory_map_count];
            *current_entry = new_entry;
            last_entry     = current_entry;
            target_boot_info->boot_memory_info.memory_map_count++;
        }

        target_boot_info->boot_memory_info.memory_size += (new_entry.page_count * EFI_PAGE_SIZE);
    }

    return EFI_SUCCESS;
}
