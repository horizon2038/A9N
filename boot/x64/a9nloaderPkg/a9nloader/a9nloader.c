#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include  <Library/PrintLib.h>
#include  <Library/MemoryAllocationLib.h>
#include  <Library/BaseMemoryLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/BlockIo.h>
#include "a9nloader.h"

// PROTOTYPE
void print_info();
void get_memory_map();
void open_root_directory(EFI_HANDLE, EFI_FILE_PROTOCOL);
void open_directory();

EFI_STATUS EFIAPI efi_main (IN EFI_HANDLE image_handle, IN EFI_SYSTEM_TABLE *system_table)
{
    EFI_STATUS status;
    EFI_LOADED_IMAGE_PROTOCOL* kernel_image;// kernel file (*.elf)
    EFI_FILE_PROTOCOL *root_directory;

    // system_table->ConOut->ClearScreen(system_table->ConOut);
    print_info(&system_table);
    while(1);
    return 0;
}

void print_info(EFI_SYSTEM_TABLE *system_table)
{
    system_table->ConOut->OutputString(system_table->ConOut, L"a9nloader");
}

void open_root_directory(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL **root_directory)
{
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system;
    EFI_FILE_PROTOCOL *root_directory;
    EFI_STATUS efi_status;

    efi_status = gBS->OpenProtocol(image_handle, &gEfiLoadedImageProtocolGuid, (VOID**)&file_system, image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if(EFI_ERROR(efi_status))
    {
        return efi_status;
    }
    efi_status = file_system->OpenVolume(file_system, &root_directory);
}