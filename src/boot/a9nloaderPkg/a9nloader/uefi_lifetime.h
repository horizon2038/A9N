#ifndef UEFI_LIFETIME_H
#define UEFI_LIFETIME_H

#include "uefi_memory_map.h"

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Uefi.h>

EFI_STATUS
exit_uefi(EFI_HANDLE image_handle, uefi_memory_map *target_uefi_memory_map);

#endif
