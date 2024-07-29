#ifndef UEFI_BOOT_INFO_CONFIGURATOR
#define UEFI_BOOT_INFO_CONFIGURATOR

#include <Uefi.h>

#include "boot_info.h"
#include "uefi_memory_map.h"

EFI_STATUS make_boot_info(
    EFI_SYSTEM_TABLE *system_table,
    uefi_memory_map  *target_uefi_memory_map,
    boot_info        *target_boot_info
);

#endif
