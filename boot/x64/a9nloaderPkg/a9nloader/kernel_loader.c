#include "kernel_loader.h"

#include <Library/UefiBootServicesTableLib.h>
#include <Uefi.h>
#include <Library/UefiLib.h>

#include "file_reader.h"
#include "elf_info_logger.h"

EFI_STATUS load_elf_header(EFI_FILE_PROTOCOL*, elf64_header*);

EFI_STATUS load_kernel(EFI_FILE_PROTOCOL *kernel, uint64_t offset)
{
    EFI_STATUS efi_status;
    elf64_header loaded_elf64_header = (elf64_header) {0};
    load_elf_header(kernel, &loaded_elf64_header);
    // efi_status = read_file(kernel, 0, sizeof(elf64_header), (void*)&loaded_elf64_header);
    print_elf_info(&loaded_elf64_header);
    efi_status = calculate_elf_segment(&loaded_elf64_header, 0);
    // elf64_program_header *loaded_elf64_program_header;
    return efi_status;
}

EFI_STATUS load_elf_header(EFI_FILE_PROTOCOL *kernel, elf64_header *header)
{
    EFI_STATUS efi_status;
    efi_status = read_file(kernel, 0, sizeof(elf64_header), (void*)header);
    return efi_status;
}

EFI_STATUS calculate_elf_segment(elf64_header *header, EFI_PHYSICAL_ADDRESS image_base)
{
    EFI_STATUS efi_status;

    // Get pointer to program headers
    // efi_status = read_file();
    elf64_program_header *program_header = (elf64_program_header *)((UINTN)header + header->program_header_offset);
    Print(L"start_calculate_elf_segment\r\n");

    // Load each program segment into memory
    for (UINT16 i = 0; i < header->program_header_number; i++)
    {
        // Loadable segment type
        if (program_header[i].type != PT_LOAD)
        {
            Print(L"PT_LOAD\r\n");
            continue;
        }
        Print(L"count: %d", i);

        efi_status = load_elf_segment(header, program_header, 0x000000);
    }
    return efi_status;
}

EFI_STATUS load_elf_segment(elf64_header *header, elf64_program_header *program_header, EFI_PHYSICAL_ADDRESS segment_start)
{
    EFI_STATUS efi_status;
    efi_status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, EFI_SIZE_TO_PAGES(program_header->memory_size), &segment_start);
    gBS->CopyMem((VOID *)(UINTN)segment_start, (VOID *)((UINTN)header + program_header->offset), program_header->file_size);
    return efi_status;
}

void zero_clear(elf64_program_header *program_header, EFI_PHYSICAL_ADDRESS segment_start, uint16_t count)
{
    if (program_header->file_size < program_header->memory_size)
    {
        gBS->SetMem((VOID *)(UINTN)(segment_start + program_header->file_size), program_header->memory_size - program_header->file_size, 0);
        // if (EFI_ERROR(efi_status))
        // {
        //     Print(L"Failed to zero out remaining memory in segment %d: %r\n", count, efi_status);
        // }
    }
}

