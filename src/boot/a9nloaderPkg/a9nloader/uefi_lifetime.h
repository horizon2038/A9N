#ifndef UEFI_LIFETIME_H
#define UEFI_LIFETIME_H

#include "memory_map.h"

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>


EFI_STATUS exit_uefi(EFI_HANDLE image_handle);

#endif
