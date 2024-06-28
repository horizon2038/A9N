#include "kernel_opener.h" // impl

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>

static EFI_STATUS open_root_directory(EFI_HANDLE, EFI_FILE_PROTOCOL **);
static EFI_STATUS get_image(EFI_HANDLE, EFI_LOADED_IMAGE_PROTOCOL **);
static EFI_STATUS
    get_root_file_system(EFI_HANDLE, EFI_HANDLE, EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **);
static EFI_STATUS
    get_root_directory(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *, EFI_FILE_PROTOCOL **);

EFI_STATUS open_kernel(
    EFI_HANDLE          image_handle,
    EFI_FILE_PROTOCOL **root_directory,
    EFI_FILE_PROTOCOL **kernel
)
{
    EFI_STATUS efi_status;

    efi_status = open_root_directory(image_handle, root_directory);
    (*root_directory)
        ->Open(
            *root_directory,
            kernel,
            u"kernel\\kernel.elf",
            EFI_FILE_MODE_READ,
            EFI_FILE_READ_ONLY
        );
    return efi_status;
}

static EFI_STATUS open_root_directory(
    EFI_HANDLE          image_handle,
    EFI_FILE_PROTOCOL **root_directory
)
{
    EFI_LOADED_IMAGE_PROTOCOL       *device_image;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system;
    EFI_STATUS                       efi_status = EFI_SUCCESS;

    efi_status = get_image(image_handle, &device_image);
    if (EFI_ERROR(efi_status))
        return efi_status;
    efi_status = get_root_file_system(
        image_handle,
        device_image->DeviceHandle,
        &file_system
    );
    if (EFI_ERROR(efi_status))
        return efi_status;
    efi_status = get_root_directory(file_system, root_directory);
    if (EFI_ERROR(efi_status))
        return efi_status;

    return efi_status;
}

static EFI_STATUS
    get_image(EFI_HANDLE image_handle, EFI_LOADED_IMAGE_PROTOCOL **device_image)
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
