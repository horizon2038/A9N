#include "file_info_logger.h"

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>
#include "stdint.h"

static uint64_t calculate_file_size();
static EFI_STATUS
            get_file_info(EFI_FILE_PROTOCOL **, uint64_t *, EFI_FILE_INFO *);
static void print_info(EFI_FILE_INFO *);

EFI_STATUS print_file_info(EFI_FILE_PROTOCOL **file)
{
    uint64_t      file_size;
    EFI_FILE_INFO file_info;
    EFI_STATUS    efi_status;
    file_size  = calculate_file_size();
    efi_status = get_file_info(file, &file_size, &file_info);
    print_info(&file_info);
    return efi_status;
}

static uint64_t calculate_file_size()
{
    uint64_t file_size;
    file_size = sizeof(EFI_FILE_INFO);
    file_size += sizeof(short) * 16;
    return file_size;
}

static EFI_STATUS get_file_info(
    EFI_FILE_PROTOCOL **file,
    uint64_t           *file_size,
    EFI_FILE_INFO      *file_info
)
{
    EFI_STATUS efi_status;
    efi_status
        = (*file)
              ->GetInfo(*file, &gEfiFileInfoGuid, file_size, (VOID *)file_info);
    return efi_status;
}

static void print_info(EFI_FILE_INFO *file_info)
{
    Print(
        L"[ INFO ] file_name: %s\r\n[ INFO ] file_size: %llu bytes\r\n[ INFO ] "
        L"file_physical_size: %llu bytes\r\n",
        file_info->FileName,
        file_info->FileSize,
        file_info->PhysicalSize
    );
    Print(L"\r\n");
}
