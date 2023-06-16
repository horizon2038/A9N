#ifndef FILE_INFO_LOGGER_H
#define FILE_INFO_LOGGER_H

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Protocol/SimpleFileSystem.h>

EFI_STATUS print_file_info(EFI_FILE_PROTOCOL**);

#endif