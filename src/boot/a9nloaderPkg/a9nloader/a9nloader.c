#include "a9nloader.h"

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

#include "elf.h"    
#include "kernel_opener.h"
#include "file_info_logger.h"
#include "kernel_loader.h"
#include "error_handler.h"
#include "kernel_jumper.h"
#include "uefi_lifetime.h"

#include "uefi_memory_map.h"
#include "boot_info.h"
#include "uefi_boot_info_configurator.h"

EFI_STATUS EFIAPI efi_main (IN EFI_HANDLE image_handle, IN EFI_SYSTEM_TABLE *system_table)
{
    EFI_STATUS efi_status = EFI_SUCCESS;
    EFI_FILE_PROTOCOL *root_directory;
    EFI_FILE_PROTOCOL *kernel;
    uint64_t entry_point_address = 0;

    uefi_memory_map target_uefi_memory_map;
    boot_info target_boot_info;

    system_table->ConOut->ClearScreen(system_table->ConOut);
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_WHITE);
    Print(L"[ INFO ] a9nloader v0.0.1\r\n");
    Print(L"[ RUN ] efi_main\r\n");
    Print(L"\r\n");

    efi_status = open_kernel(image_handle, &root_directory, &kernel);
    if(EFI_ERROR(efi_status)) return efi_status;
    efi_status = print_file_info(&kernel);
    if(EFI_ERROR(efi_status)) return efi_status;
    efi_status = load_kernel(kernel, &entry_point_address);
    if(EFI_ERROR(efi_status)) return efi_status;
    efi_status = get_uefi_memory_map(&target_uefi_memory_map);
    if(EFI_ERROR(efi_status)) return efi_status;
    efi_status = make_boot_info(&target_uefi_memory_map, &target_boot_info);
    if(EFI_ERROR(efi_status)) return efi_status;
    efi_status = exit_uefi(image_handle, &target_uefi_memory_map);
    // known issues: checking efi_status in exit_uefi causes "EFI Hard Drive" error.

    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_GREEN);
    
    jump_kernel(entry_point_address, &target_boot_info);

    while(1);
    return efi_status;
}
