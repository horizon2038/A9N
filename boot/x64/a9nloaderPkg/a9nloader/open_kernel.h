#ifndef OPEN_KERNEL_H
#define OPEN_KERNEL_H

#include <Uefi.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>

EFI_STATUS open_root_directory(EFI_HANDLE, EFI_FILE_PROTOCOL**);
EFI_STATUS get_image(EFI_HANDLE, EFI_LOADED_IMAGE_PROTOCOL**);
EFI_STATUS get_root_file_system(EFI_HANDLE, EFI_HANDLE, EFI_SIMPLE_FILE_SYSTEM_PROTOCOL**);
EFI_STATUS get_root_directory(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL*, EFI_FILE_PROTOCOL**);

#endif