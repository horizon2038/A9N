#include "kernel_loader.h"

#include <Library/UefiBootServicesTableLib.h>

#include "file_reader.h"

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

EFI_STATUS calculate_elf_segment(elf64_header *header, EFI_PHYSICAL_ADDRESS image_base)
{
    EFI_STATUS status;

    // Get pointer to program headers
    elf64_program_header *program_header = (elf64_program_header *)((UINTN)header + header->program_header_offset);

    // Load each program segment into memory
    for (UINT16 i = 0; i < header->program_header_number; i++)
    {
        // Loadable segment type
        if (program_header[i].type != PT_LOAD)
        {
            continue;
        }

        load_elf_segment(header, program_header, 0x000000);
    }
}

EFI_STATUS load_elf_segment(elf64_header *header, elf64_program_header *program_header, EFI_PHYSICAL_ADDRESS segment_start)
{
    EFI_STATUS efi_status;
    efi_status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, EFI_SIZE_TO_PAGES(program_header->memory_size), &segment_start);
    gBS->CopyMem((VOID *)(UINTN)segment_start, (VOID *)((UINTN)header + program_header->offset), program_header->file_size);
    return efi_status;
}

EFI_STATUS zero_clear(elf64_program_header *program_header, EFI_PHYSICAL_ADDRESS segment_start, uint16_t count)
{
    EFI_STATUS efi_status;
    if (program_header->file_size < program_header->memory_size)
    {
        gBS->SetMem((VOID *)(UINTN)(segment_start + program_header->file_size), program_header->memory_size - program_header->file_size, 0);
        if (EFI_ERROR(efi_status))
        {
            Print(L"Failed to zero out remaining memory in segment %d: %r\n", count, efi_status);
            return efi_status;
        }
        return efi_status;
    }
    return efi_status;
}

