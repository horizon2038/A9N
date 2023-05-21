#ifndef FILE_INFO_LOGGER_H
#define FILE_INFO_LOGGER_H

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

EFI_STATUS print_file_info(EFI_FILE_PROTOCOL**);
UINT64 calculate_file_size();
EFI_STATUS get_file_info(EFI_FILE_PROTOCOL **, UINT64*, EFI_FILE_INFO*);
void print_info(EFI_FILE_INFO*);

#endif