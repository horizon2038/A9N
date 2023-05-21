#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/BlockIo.h>
#include <Guid/FileInfo.h>

#include "a9nloader.h"
#include "elf.h"
#include "open_kernel.h"

EFI_STATUS EFIAPI efi_main (IN EFI_HANDLE image_handle, IN EFI_SYSTEM_TABLE *system_table)
{
    system_table->ConOut->ClearScreen(system_table->ConOut);
    Print(L"start_efi_main\r\n");
    EFI_STATUS efi_status;
    EFI_FILE_PROTOCOL *root_directory;
    EFI_FILE_PROTOCOL *kernel;
    efi_status = open_root_directory(image_handle, &root_directory);
    root_directory->Open(root_directory, &kernel, u"kernel\\kernel.elf", EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
    EFI_FILE_INFO file_info;
    UINT64 file_size;
    file_size = sizeof(file_info) + sizeof(CHAR16) * 16;
    efi_status = kernel->GetInfo(kernel, &gEfiFileInfoGuid, &file_size, (VOID*)&file_info);
    Print(L"file_name: %s\nfile_size: %llu bytes\nfile_physical_size: %llu bytes", file_info.FileName, (unsigned long long)file_info.FileSize, (unsigned long long)file_info.PhysicalSize);
    while(1);
    return efi_status;
}