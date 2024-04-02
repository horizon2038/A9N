#ifndef OPEN_KERNEL_H
#define OPEN_KERNEL_H

#include <Uefi.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>

EFI_STATUS open_kernel(EFI_HANDLE, EFI_FILE_PROTOCOL **, EFI_FILE_PROTOCOL **);

#endif