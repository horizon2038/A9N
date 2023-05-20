#include "open_kernel.h" // impl

#include <Library/UefiBootServicesTableLib.h>

#include "error_handler.h"

EFI_STATUS open_root_directory(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL **root_directory)
{
    Print(L"start_open_root_directory\r\n");
    EFI_LOADED_IMAGE_PROTOCOL *device_image;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system;
    EFI_STATUS efi_status;
    efi_status = EFI_SUCCESS;

    efi_status = get_image(image_handle, &device_image);
    Print(L"end_get_image\r\n");
    efi_status = get_root_file_system(image_handle, device_image->DeviceHandle, &file_system);
    Print(L"end_get_root_file_system\r\n");
    efi_status = get_root_directory(file_system, root_directory);
    Print(L"end_open_root_directory\r\n");
    return efi_status;
}

EFI_STATUS get_image(EFI_HANDLE image_handle, EFI_LOADED_IMAGE_PROTOCOL **device_image)
{
    EFI_STATUS efi_status;
    efi_status = gBS->OpenProtocol(image_handle, &gEfiLoadedImageProtocolGuid, (VOID**)device_image, image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
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