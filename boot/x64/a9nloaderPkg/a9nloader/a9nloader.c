#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/BlockIo.h>
#include <Guid/FileInfo.h>

#include "a9nloader.h"
#include "elf.h"

// PROTOTYPE
EFI_STATUS open_root_directory(EFI_HANDLE, EFI_FILE_PROTOCOL**);
EFI_STATUS get_image(EFI_HANDLE, EFI_LOADED_IMAGE_PROTOCOL**);
EFI_STATUS get_root_file_system(EFI_HANDLE, EFI_HANDLE, EFI_SIMPLE_FILE_SYSTEM_PROTOCOL**);
EFI_STATUS get_root_directory(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL*, EFI_FILE_PROTOCOL**);
EFI_STATUS handle_error(EFI_STATUS);

EFI_STATUS EFIAPI efi_main (IN EFI_HANDLE image_handle, IN EFI_SYSTEM_TABLE *system_table)
{
    Print(L"start_efi_main\r\n");
    EFI_STATUS efi_status;
    EFI_FILE_PROTOCOL *root_directory;
    EFI_FILE_PROTOCOL *kernel;
    efi_status = open_root_directory(image_handle, &root_directory);

    root_directory->Open(root_directory, &kernel, L"EFI\\BOOT\\BOOTX64.EFI", EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
    EFI_FILE_INFO file_info;
    UINT64 file_size;
    file_size = sizeof(file_info);
    efi_status = kernel->GetInfo(kernel, &gEfiFileInfoGuid, &file_size, (VOID*)&file_info);
    Print(L"file_name: %s\nfile_size: %llu (%llu on disk) bytes\r\n", file_info.FileName, (unsigned long long)file_info.FileSize, (unsigned long long)file_info.PhysicalSize);

    while(1);
    return efi_status;
}

EFI_STATUS open_root_directory(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL **root_directory)
{
    Print(L"start_open_root_directory\r\n");
    EFI_LOADED_IMAGE_PROTOCOL *device_image;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system;
    EFI_STATUS efi_status;
    efi_status = EFI_SUCCESS;

    efi_status = get_image(image_handle, &device_image);
    Print(L"end_get_image\r\n");
    efi_status = get_root_file_system(image_handle, device_image->DeviceHandle, &file_system);
    Print(L"end_get_root_file_system\r\n");
    efi_status = get_root_directory(file_system, root_directory);
    Print(L"end_open_root_directory\r\n");
    return efi_status;
}

EFI_STATUS get_image(EFI_HANDLE image_handle, EFI_LOADED_IMAGE_PROTOCOL **device_image)
{
    EFI_STATUS efi_status;
    efi_status = gBS->OpenProtocol(image_handle, &gEfiLoadedImageProtocolGuid, (VOID**)device_image, image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    return handle_error(efi_status);
}

EFI_STATUS get_root_file_system(EFI_HANDLE image_handle, EFI_HANDLE device_handle, EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **file_system)
{
    EFI_STATUS efi_status;
    efi_status = gBS->OpenProtocol(device_handle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)file_system, image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    return handle_error(efi_status);
}

EFI_STATUS get_root_directory(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* root_file_system, EFI_FILE_PROTOCOL **root_directory)
{
    EFI_STATUS efi_status;
    efi_status = root_file_system->OpenVolume(root_file_system, root_directory);
    return handle_error(efi_status);
}

EFI_STATUS handle_error(EFI_STATUS efi_status)
{
    if(EFI_ERROR(efi_status))
    {
        Print(L"EFI_ERROR\r\n");
        return efi_status;
    }
    return EFI_SUCCESS;
}

EFI_STATUS load_kernel(EFI_FILE_PROTOCOL *kernel, uint64_t offset)
{
    elf64_header *loaded_elf64_header;
    // load header
    elf64_program_header *loaded_elf64_program_header;
    // read header.
    // calculate elf_location
    // locatiion(load) elf segment.
    // complete.

}

uint64_t calculate_file_size(EFI_FILE_PROTOCOL *file, uint64_t *file_size)
{
    EFI_STATUS efi_status;
    EFI_FILE_INFO *file_info;

    *file_size = sizeof(file_info) + sizeof(uint16_t) * 256;
    efi_status = file->GetInfo(file, &gEfiFileInfoGuid, file_size, (VOID*)file_info);
    return file_info->FileSize;
}

EFI_STATUS read_file(EFI_FILE_PROTOCOL *file, uint64_t offset, int size, void **buffer)
{
    EFI_STATUS efi_status;
    // efi_status = gBS->AllocatePool(EfiLoaderData, size, buffer);
    *buffer = AllocatePool(size);
    efi_status = file->SetPosition(file, offset);
    efi_status =  file->Read(file, &size, *buffer);
    return efi_status;
}

EFI_STATUS calculate_elf_segment(elf64_header *elf64_header, elf64_address head)
{
    elf64_program_header *loaded_elf64_program_header;
    loaded_elf64_program_header = (elf64_program_header*)(head + elf64_header->program_header_offset);
    for(int i=0; i < elf64_header->program_header_number; i++)
    {
        if(loaded_elf64_program_header[i].type != PT_LOAD)
        {
            continue; // early return
        }
        // TODO:
        // load segment: PT_LOAD to memory.
        // process
    }
}

EFI_STATUS calculate_elf_segment(elf64_header *elf64_header, EFI_PHYSICAL_ADDRESS image_base)
{
    EFI_STATUS status;

    // Get pointer to program headers
    elf64_program_header *program_header = (elf64_program_header *)((UINTN)elf64_header + elf64_header->program_header_offset);

    // Load each program segment into memory
    for (UINT16 i = 0; i < elf64_header->program_header_number; i++)
    {
        // Loadable segment type
        if (program_header[i].type != PT_LOAD)
        {
            continue;
        }
        // Get the start address of the segment in memory
        EFI_PHYSICAL_ADDRESS segment_start = image_base + program_header[i].virtual_address;
        // Allocate memory for the segment
        status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, EFI_SIZE_TO_PAGES(program_header[i].memory_size), &segment_start);
        if (EFI_ERROR(status))
        {
            Print(L"Failed to allocate memory for segment %d: %r\n", i, status);
            return status;
        }
        // Copy the segment from the file to memory
        gBS->CopyMem((VOID *)(UINTN)segment_start, (VOID *)((UINTN)elf64_header + program_header[i].offset), program_header[i].file_size);
        if (EFI_ERROR(status))
        {
            Print(L"Failed to copy segment %d to memory: %r\n", i, status);
            return status;
        }
        // Zero out any remaining memory in the segment
        if (program_header[i].file_size < program_header[i].memory_size)
        {
            gBS->SetMem((VOID *)(UINTN)(segment_start + program_header[i].file_size), program_header[i].memory_size - program_header[i].file_size, 0);
            if (EFI_ERROR(status))
            {
                Print(L"Failed to zero out remaining memory in segment %d: %r\n", i, status);
                return status;
            }
        }
    }
}

EFI_STATUS load_elf_segment(elf64_header *header, elf64_program_header *program_header, EFI_PHYSICAL_ADDRESS segment_start)
{
    EFI_STATUS efi_status;
    efi_status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, EFI_SIZE_TO_PAGES(program_header->memory_size), &segment_start);
    gBS->CopyMem((VOID *)(UINTN)segment_start, (VOID *)((UINTN)header + program_header->offset), program_header->file_size);
    return efi_status;
}

EFI_STATUS zero_clear(elf64_program_header *program_header, EFI_PHYSICAL_ADDRESS segment_start)
{
    EFI_STATUS efi_status;
    if (program_header->file_size < program_header->memory_size)
    {
        gBS->SetMem((VOID *)(UINTN)(segment_start + program_header->file_size), program_header->memory_size - program_header->file_size, 0);
        if (EFI_ERROR(efi_status))
        {
            Print(L"Failed to zero out remaining memory in segment %d: %r\n", i, efi_status);
            return efi_status;
        }
        return efi_status;
    }
    return efi_status;
    // TODO: Connection all things.
}

// EFI_STATUS load_elf_segment();
