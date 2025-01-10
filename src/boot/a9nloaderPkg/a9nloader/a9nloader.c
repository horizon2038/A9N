#include "a9nloader.h"

#include <Guid/FileInfo.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Uefi.h>

#include "boot_info.h"
#include "boot_info_allocator.h"
#include "elf.h"
#include "error_handler.h"
#include "file_info_logger.h"
#include "kernel_jumper.h"
#include "kernel_loader.h"
#include "kernel_opener.h"
#include "uefi_boot_info_configurator.h"
#include "uefi_lifetime.h"
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
    system_table->ConOut
        ->SetAttribute(system_table->ConOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_BACKGROUND_BLACK));
    Print(L"[ INFO ] a9nloader v0.2.1\r\n");
    Print(L"[ RUN ] efi_main\r\n");
    Print(L"\r\n");

    efi_status = open_root_directory(image_handle, &root_directory);
    if (EFI_ERROR(efi_status))
    {
        Print(L"[ ERROR ] failed to open_root_directory\r\n");
        return efi_status;
    }

    // open and load kernel
    efi_status = open_kernel(root_directory, &kernel);
    if (EFI_ERROR(efi_status))
    {
        Print(L"[ ERROR ] failed to open_kernel\r\n");
        return efi_status;
    }

    efi_status = print_file_info(&kernel);
    if (EFI_ERROR(efi_status))
    {
        Print(L"[ ERROR ] failed to print_file_info (kernel)\r\n");
        return efi_status;
    }
    Print(L"start load kernel ...\r\n");

    efi_status = load_kernel(kernel, &kernel_entry_point);
    if (EFI_ERROR(efi_status))
    {
        Print(L"[ ERROR ] failed to load_kernel\r\n");
        return efi_status;
    }

    // open and load init server
    efi_status = open_init_server(root_directory, &init_server);
    if (EFI_ERROR(efi_status))
    {
        Print(L"[ ERROR ] failed to open_init_server\r\n");
        return efi_status;
    }

    efi_status = print_file_info(&init_server);
    if (EFI_ERROR(efi_status))
    {
        Print(L"[ ERROR ] failed to print_file_info (init_server)\r\n");
        return efi_status;
    }

    efi_status = load_init_server(
        init_server,
        &target_boot_info.boot_init_image_info.loaded_address,
        &target_boot_info.boot_init_image_info.init_image_size,
        &target_boot_info.boot_init_image_info.entry_point_address,
        &target_boot_info.boot_init_image_info.init_info_address,
        &target_boot_info.boot_init_image_info.init_ipc_buffer_address
    );
    if (EFI_ERROR(efi_status))
    {
        Print(L"[ ERROR ] failed to load_init_server\r\n");
        return efi_status;
    }

    // boot_info must be allocated before getting UEFI memory map
    efi_status = allocate_boot_info(&target_boot_info);
    if (EFI_ERROR(efi_status))
    {
        Print(L"[ ERROR ] failed to allocate memory_info\r\n");
        return efi_status;
    }

    // common
    efi_status = get_uefi_memory_map(&target_uefi_memory_map);
    if (EFI_ERROR(efi_status))
    {
        Print(L"[ ERROR ] failed to get uefi memory map\r\n");
        return efi_status;
    }

    efi_status = make_boot_info(system_table, &target_uefi_memory_map, &target_boot_info);
    if (EFI_ERROR(efi_status))
    {
        Print(L"[ ERROR ] failed to make boot info\r\n");
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
