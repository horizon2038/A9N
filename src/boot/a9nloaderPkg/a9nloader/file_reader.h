#ifndef READ_FILE_H
#define READ_FILE_H

#include <Library/UefiLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Uefi.h>
#include <stdint.h> // Standard Library in EDK2 ?

EFI_STATUS
    read_file(EFI_FILE_PROTOCOL *file, uint64_t offset, uint64_t size, void **buffer);
#endif
