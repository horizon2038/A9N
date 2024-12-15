#ifndef BOOT_INFO_ALLOCATOR_H
#define BOOT_INFO_ALLOCATOR_H

#include "boot_info.h"
#include <Uefi.h>

EFI_STATUS allocate_boot_info(boot_info *info);

#endif
