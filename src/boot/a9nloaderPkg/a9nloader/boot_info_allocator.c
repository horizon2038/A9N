#include "boot_info_allocator.h"

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Uefi.h>

static const UINTN RESERVED_MEMORY_INFO_PAGE_COUNT = 2;

EFI_STATUS allocate_boot_info(boot_info *info)
{
    if (!info)
    {
        return EFI_NOT_FOUND;
    }

    EFI_PHYSICAL_ADDRESS memory_info_map_address = 0;

    EFI_STATUS efi_status                        = gBS->AllocatePages(
        AllocateAnyPages,
        EfiReservedMemoryType,
        RESERVED_MEMORY_INFO_PAGE_COUNT,
        &memory_info_map_address
    );
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }
    info->boot_memory_info.memory_map = (memory_map_entry *)memory_info_map_address;

    return EFI_SUCCESS;
}
