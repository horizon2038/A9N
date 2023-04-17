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
void open_root_directory(EFI_HANDLE, EFI_FILE_PROTOCOL**);
EFI_STATUS get_image(EFI_HANDLE, EFI_LOADED_IMAGE_PROTOCOL**);
EFI_STATUS get_root_file_system(EFI_HANDLE, EFI_HANDLE, EFI_SIMPLE_FILE_SYSTEM_PROTOCOL**);
EFI_STATUS handle_error(EFI_STATUS);


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

void aiueo_open_root_directory(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL **root_directory)
{
    EFI_LOADED_IMAGE_PROTOCOL *device_image;
    EFI_STATUS efi_status;

    efi_status = gBS->OpenProtocol(image_handle, &gEfiLoadedImageProtocolGuid, (VOID**)&device_image, image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if(EFI_ERROR(efi_status))
    {
        return efi_status;
    }
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system;
    efi_status = gBS->OpenProtocol(device_image->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)file_system, image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if(EFI_ERROR(efi_status))
    {
        return efi_status;
    }
    return file_system->OpenVolume(file_system, root_directory); // void
}

void open_root_directory(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL **root_directory)
{
    EFI_LOADED_IMAGE_PROTOCOL *device_image;
    EFI_STATUS efi_status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system;

    efi_status = get_image(image_handle, &device_image);
    efi_status = get_root_file_system(image_handle, device_image->DeviceHandle, &file_system);
    efi_status = get_root_directory(file_system, &root_directory);
}

EFI_STATUS get_image(EFI_HANDLE image_handle, EFI_LOADED_IMAGE_PROTOCOL **device_image)
{
    EFI_STATUS efi_status;
    efi_status = gBS->OpenProtocol(image_handle, &gEfiLoadedImageProtocolGuid, (VOID**)&device_image, image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    return handle_error(efi_status);
}

EFI_STATUS get_root_file_system(EFI_HANDLE image_handle, EFI_HANDLE device_handle, EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **file_system)
{
    EFI_STATUS efi_status;
    efi_status = gBS->OpenProtocol(device_handle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)file_system, image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    return handle_error(efi_status);
}

EFI_STATUS get_root_directory(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* root_file_system, EFI_FILE_PROTOCOL **root_directory)
{
    EFI_STATUS efi_status;
    efi_status = root_file_system->OpenVolume(root_file_system, root_directory);
    return handle_error(efi_status);
}

EFI_STATUS handle_error(EFI_STATUS efi_status)
{
    if(EFI_ERROR(efi_status))
    {
        return efi_status;
    }
    return efi_status;
}
