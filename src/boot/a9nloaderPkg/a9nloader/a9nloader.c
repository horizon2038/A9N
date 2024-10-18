#include "a9nloader.h"

#include <Guid/FileInfo.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Uefi.h>

#include "elf.h"
#include "error_handler.h"
#include "file_info_logger.h"
#include "kernel_jumper.h"
#include "kernel_loader.h"
#include "kernel_opener.h"
#include "uefi_lifetime.h"

#include "boot_info.h"
#include "uefi_boot_info_configurator.h"
#include "uefi_memory_map.h"

EFI_STATUS EFIAPI efi_main(IN EFI_HANDLE image_handle, IN EFI_SYSTEM_TABLE *system_table)
{
    EFI_STATUS efi_status = EFI_SUCCESS;

    EFI_FILE_PROTOCOL *root_directory;

    // kernel
    EFI_FILE_PROTOCOL *kernel;
    uint64_t           kernel_entry_point = 0;

    // init server
    EFI_FILE_PROTOCOL *init_server;
    // uint64_t           init_server_entry_point = 0;

    // common
    uefi_memory_map target_uefi_memory_map;
    boot_info       target_boot_info;

    system_table->ConOut->ClearScreen(system_table->ConOut);
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_WHITE);
    Print(L"[ INFO ] a9nloader v0.0.1\r\n");
    Print(L"[ RUN ] efi_main\r\n");
    Print(L"\r\n");

    // open and load kernel
    efi_status = open_kernel(image_handle, &root_directory, &kernel);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    efi_status = print_file_info(&kernel);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    efi_status = load_kernel(kernel, &kernel_entry_point);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    // open and load init server
    efi_status = open_init_server(image_handle, &root_directory, &init_server);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    efi_status = print_file_info(&init_server);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    // common
    efi_status = get_uefi_memory_map(&target_uefi_memory_map);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    efi_status = make_boot_info(system_table, &target_uefi_memory_map, &target_boot_info);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    efi_status = exit_uefi(image_handle, &target_uefi_memory_map);
    // known issues: checking efi_status in exit_uefi causes "EFI Hard Drive"
    // error.

    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_GREEN);

    jump_kernel(kernel_entry_point, &target_boot_info);

    while (1)
        ;
    return efi_status;
}
