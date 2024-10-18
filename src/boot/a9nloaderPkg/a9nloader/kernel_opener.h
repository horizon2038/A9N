#ifndef OPEN_KERNEL_H
#define OPEN_KERNEL_H

#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Uefi.h>

EFI_STATUS open_root_directory(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL **root_directory);

EFI_STATUS open_kernel(EFI_FILE_PROTOCOL *root_directory, EFI_FILE_PROTOCOL **kernel_file);

EFI_STATUS open_init_server(EFI_FILE_PROTOCOL *root_directory, EFI_FILE_PROTOCOL **init_server_file);

#endif
