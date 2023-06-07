#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

#include "a9nloader.h"
#include "elf.h"    
#include "kernel_opener.h"
#include "file_info_logger.h"
#include "kernel_loader.h"
#include "error_handler.h"

EFI_STATUS EFIAPI efi_main (IN EFI_HANDLE image_handle, IN EFI_SYSTEM_TABLE *system_table)
{
    EFI_FILE_PROTOCOL *root_directory;
    EFI_FILE_PROTOCOL *kernel;
    EFI_STATUS efi_status = EFI_SUCCESS;

    system_table->ConOut->ClearScreen(system_table->ConOut);
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_WHITE);
    Print(L"a9nloader v0.0.1\r\n");
    Print(L"start_efi_main\r\n");

    while(!EFI_ERROR(efi_status))
    {
        efi_status = open_kernel(image_handle, &root_directory, &kernel);
        efi_status = print_file_info(&kernel);
        efi_status = load_kernel(kernel, 0);
        break;
    }
    efi_status = handle_error(efi_status);
    while(1);
    return efi_status;
}