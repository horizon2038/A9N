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
#include "kernel_opener.h"
#include "file_info_logger.h"
#include "kernel_loader.h"

EFI_STATUS EFIAPI efi_main (IN EFI_HANDLE image_handle, IN EFI_SYSTEM_TABLE *system_table)
{
    system_table->ConOut->ClearScreen(system_table->ConOut);
    Print(L"a9nloader v0.0.1\r\n");
    Print(L"start_efi_main\r\n");
    EFI_STATUS efi_status;
    EFI_FILE_PROTOCOL *root_directory;
    EFI_FILE_PROTOCOL *kernel;
    efi_status = open_kernel(image_handle, &root_directory, &kernel);
    efi_status = print_file_info(&kernel);
    efi_status = load_kernel(kernel, 0);
    while(1);
    return efi_status;
}