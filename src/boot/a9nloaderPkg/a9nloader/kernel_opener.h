#ifndef OPEN_KERNEL_H
#define OPEN_KERNEL_H

#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Uefi.h>

EFI_STATUS open_kernel(EFI_HANDLE, EFI_FILE_PROTOCOL **, EFI_FILE_PROTOCOL **);

#endif