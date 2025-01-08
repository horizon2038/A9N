#include "file_info_logger.h"

#include "stdint.h"
#include <Guid/FileInfo.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Uefi.h>

static uint64_t   calculate_file_size(void);
static EFI_STATUS make_file_info(EFI_FILE_INFO **info, uint64_t file_size);
static EFI_STATUS get_file_info(EFI_FILE_PROTOCOL **, uint64_t *, EFI_FILE_INFO *);
static void       print_info(EFI_FILE_INFO *);

EFI_STATUS print_file_info(EFI_FILE_PROTOCOL **file)
{
    EFI_STATUS     efi_status;
    EFI_FILE_INFO *file_info;
    uint64_t       file_size = 0;

    file_size                = calculate_file_size();
    efi_status               = make_file_info(&file_info, file_size);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    efi_status = get_file_info(file, &file_size, file_info);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    print_info(file_info);

    efi_status = gBS->FreePool(file_info);

    return efi_status;
}

static EFI_STATUS make_file_info(EFI_FILE_INFO **info, uint64_t file_size)
{
    return gBS->AllocatePool(EfiLoaderData, file_size, (void **)info);
}

static uint64_t calculate_file_size(void)
{
    uint64_t file_size;
    file_size  = sizeof(EFI_FILE_INFO);
    file_size += sizeof(short) * 64;
    return file_size;
}

static EFI_STATUS get_file_info(EFI_FILE_PROTOCOL **file, uint64_t *file_size, EFI_FILE_INFO *file_info)
{
    EFI_STATUS efi_status;
    efi_status = (*file)->GetInfo(*file, &gEfiFileInfoGuid, file_size, (VOID *)file_info);
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
