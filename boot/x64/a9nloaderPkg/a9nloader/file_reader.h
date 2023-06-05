#ifndef READ_FILE_H
#define READ_FILE_H

#include <stdint.h> // Standard Library in EDK2 ?

#include <Uefi.h>
#include <Library/UefiLib.h>
// #include <Protocol/SimpleFIleSystem.h>

EFI_STATUS read_file(EFI_FILE_PROTOCOL *file, uint64_t offset, int size, void *buffer);
EFI_STATUS read_file_fixed(EFI_FILE_PROTOCOL *file, uint64_t offset, int size, void **buffer);
#endif
