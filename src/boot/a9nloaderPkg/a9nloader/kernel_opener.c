#include "kernel_opener.h" // impl

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Uefi.h>

static EFI_STATUS get_image(EFI_HANDLE image_handle, EFI_LOADED_IMAGE_PROTOCOL **device_image);
static EFI_STATUS get_root_file_system(
    EFI_HANDLE                        image_handle,
    EFI_HANDLE                        device_handle,
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **file_system
);
static EFI_STATUS get_root_directory(
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *root_file_system,
    EFI_FILE_PROTOCOL              **root_directory
);

EFI_STATUS open_root_directory(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL **root_directory)
{
    EFI_LOADED_IMAGE_PROTOCOL       *device_image;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system;
    EFI_STATUS                       efi_status = EFI_SUCCESS;

    efi_status                                  = get_image(image_handle, &device_image);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    efi_status = get_root_file_system(image_handle, device_image->DeviceHandle, &file_system);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    efi_status = get_root_directory(file_system, root_directory);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    return efi_status;
}

EFI_STATUS
open_kernel(EFI_FILE_PROTOCOL *root_directory, EFI_FILE_PROTOCOL **kernel_file)
{
    root_directory
        ->Open(root_directory, kernel_file, u"kernel\\kernel.elf", EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);

    return EFI_SUCCESS;
}

EFI_STATUS
open_init_server(EFI_FILE_PROTOCOL *root_directory, EFI_FILE_PROTOCOL **init_server_file)
{
    root_directory->Open(
        root_directory,
        init_server_file,
        u"kernel\\init.elf",
        EFI_FILE_MODE_READ,
        EFI_FILE_READ_ONLY
    );

    return EFI_SUCCESS;
}

static EFI_STATUS get_image(EFI_HANDLE image_handle, EFI_LOADED_IMAGE_PROTOCOL **device_image)
{
    EFI_STATUS efi_status;

    efi_status = gBS->OpenProtocol(
        image_handle,
        &gEfiLoadedImageProtocolGuid,
        (VOID **)device_image,
        image_handle,
        NULL,
        EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
    );
    return efi_status;
}

static EFI_STATUS get_root_file_system(
    EFI_HANDLE                        image_handle,
    EFI_HANDLE                        device_handle,
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **file_system
)
{
    EFI_STATUS efi_status;

    efi_status = gBS->OpenProtocol(
        device_handle,
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID **)file_system,
        image_handle,
        NULL,
        EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
    );
    return efi_status;
}

static EFI_STATUS get_root_directory(
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *root_file_system,
    EFI_FILE_PROTOCOL              **root_directory
)
{
    EFI_STATUS efi_status;

    efi_status = root_file_system->OpenVolume(root_file_system, root_directory);
    return efi_status;
}
