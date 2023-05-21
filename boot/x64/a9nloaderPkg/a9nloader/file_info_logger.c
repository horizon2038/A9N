#include "file_info_logger.h"

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

EFI_STATUS print_file_info(EFI_FILE_PROTOCOL **file)
{
    UINT64 file_size;
    EFI_FILE_INFO file_info;
    EFI_STATUS efi_status;
    file_size = calculate_file_size();
    efi_status = get_file_info(file, &file_size, &file_info);
    print_info(&file_info);
    return efi_status;
}

UINT64 calculate_file_size()
{
    UINT64 file_size;
    file_size = sizeof(EFI_FILE_INFO);
    file_size += sizeof(CHAR16) * 16;
    return file_size;
}

EFI_STATUS get_file_info(EFI_FILE_PROTOCOL **file, UINT64 *file_size, EFI_FILE_INFO *file_info)
{
    EFI_STATUS efi_status;
    efi_status = (*file)->GetInfo(*file, &gEfiFileInfoGuid, file_size, (VOID*)file_info);
    return efi_status;
}

void print_info(EFI_FILE_INFO *file_info)
{
    Print
    (
        L"file_name: %s\nfile_size: %llu bytes\nfile_physical_size: %llu bytes", 
        file_info->FileName, 
        (unsigned long long)file_info->FileSize, 
        (unsigned long long)file_info->PhysicalSize
    );
}