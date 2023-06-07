#include "file_reader.h"

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS read_file(EFI_FILE_PROTOCOL *file, uint64_t offset, int size, void **buffer)
{
    EFI_STATUS efi_status = EFI_SUCCESS;

    while(!EFI_ERROR(efi_status))
    {
        efi_status = gBS->AllocatePool(EfiReservedMemoryType, size, buffer);
        efi_status = file->SetPosition(file, offset);
        efi_status =  file->Read(file, (UINTN*)&size, *buffer);
        break;
    }
    return efi_status;
}