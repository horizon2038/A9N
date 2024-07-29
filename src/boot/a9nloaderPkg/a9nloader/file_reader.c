#include "file_reader.h"

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Uefi.h>

EFI_STATUS
    read_file(EFI_FILE_PROTOCOL *file, uint64_t offset, uint64_t size, void **buffer)
{
    EFI_STATUS efi_status = EFI_SUCCESS;

    efi_status            = gBS->AllocatePool(
        EfiLoaderData,
        size,
        buffer
    ); // EfiReservedMemoryType
    if (EFI_ERROR(efi_status))
        return efi_status;
    efi_status = file->SetPosition(file, offset);
    if (EFI_ERROR(efi_status))
        return efi_status;
    efi_status = file->Read(file, (uint64_t *)&size, *buffer);

    return efi_status;
}
