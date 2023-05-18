#include "read_file.h"

EFI_STATUS read_file(EFI_FILE_PROTOCOL *file, uint64_t offset, int size, void **buffer)
{
    EFI_STATUS efi_status;
    // efi_status = gBS->AllocatePool(EfiLoaderData, size, buffer);
    *buffer = AllocatePool(size);
    efi_status = file->SetPosition(file, offset);
    efi_status =  file->Read(file, &size, *buffer);
    return efi_status;
}