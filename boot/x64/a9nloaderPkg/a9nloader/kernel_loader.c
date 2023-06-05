#include "kernel_loader.h"

#include <Library/UefiBootServicesTableLib.h>
#include <Uefi.h>
#include <Library/UefiLib.h>

#include "file_reader.h"
#include "elf_info_logger.h"

EFI_STATUS load_elf_header(EFI_FILE_PROTOCOL*, elf64_header**);
EFI_STATUS load_elf_program_header(EFI_FILE_PROTOCOL*, elf64_header*, elf64_program_header**);

EFI_STATUS load_kernel(EFI_FILE_PROTOCOL *kernel, uint64_t offset)
{
    EFI_STATUS efi_status;
    // elf64_header loaded_elf64_header = (elf64_header) {0};
    elf64_header *loaded_elf64_header;
    efi_status = load_elf_header(kernel, &loaded_elf64_header);
    print_elf_info(loaded_elf64_header);
    elf64_program_header *loaded_elf64_program_header;
    efi_status = load_elf_program_header(kernel, loaded_elf64_header, &loaded_elf64_program_header);
    Print(L"program_header_table_physical_address: %llx\n", (*(loaded_elf64_program_header + 2)).physical_address);
    Print(L"program_header_table_physical_address: %llx\n", (loaded_elf64_program_header[2]).physical_address);
    efi_status = calculate_elf_segment(loaded_elf64_header, loaded_elf64_program_header, 0);
    return efi_status;
}

EFI_STATUS load_elf_header(EFI_FILE_PROTOCOL *kernel, elf64_header **header)
{
    EFI_STATUS efi_status;
    // efi_status = read_file(kernel, 0, sizeof(elf64_header), (void*)header);
    efi_status = read_file_fixed(kernel, 0, sizeof(elf64_header), (void**)header);
    return efi_status;
}

EFI_STATUS load_elf_program_header(EFI_FILE_PROTOCOL *kernel, elf64_header *header, elf64_program_header **program_header)
{
    EFI_STATUS efi_status;
    // gBS->AllocatePool(EfiReservedMemoryType, sizeof(elf64_program_header) * header->program_header_number, (void*)program_header);
    // efi_status = read_file(kernel, (uint64_t)header, sizeof(elf64_program_header) * header->program_header_number, (void*)program_header);
    uint64_t program_header_table_size = header->program_header_size * header->program_header_number;
    Print(L"program_header_table_size: %llx\n", program_header_table_size);
    efi_status = read_file_fixed(kernel, header->program_header_offset, program_header_table_size, (void**)program_header);
    return efi_status;
}

EFI_STATUS calculate_elf_segment(elf64_header *header, elf64_program_header *program_header_table, EFI_PHYSICAL_ADDRESS image_base)
{
    EFI_STATUS efi_status;
    Print(L"start_calculate_elf_segment\n");
    for (UINT64 i = 0; i < header->program_header_number; i++)
    {
        Print(L"count: %d\n", i);
        if ((program_header_table[i]).type != PT_LOAD)
        {
            Print(L"!PT_LOAD\r\n");
            continue;
        }
        Print(L"PT_LOAD\n");
        efi_status = load_elf_segment(header, (elf64_program_header*)&(program_header_table[i]));
        zero_clear(&(program_header_table[i]));
    }
    return efi_status;
}

EFI_STATUS load_elf_segment(elf64_header *header, elf64_program_header *program_header)
{
    Print(L"load_elf_segment\r\n");
    EFI_STATUS efi_status;
    efi_status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, EFI_SIZE_TO_PAGES(program_header->memory_size), (EFI_PHYSICAL_ADDRESS*)program_header->virtual_address);
    gBS->CopyMem((void *)program_header->physical_address, (void *)((UINTN)header + program_header->offset), program_header->file_size);
    return efi_status;
}

void zero_clear(elf64_program_header *program_header)
{
    if (program_header->file_size < program_header->memory_size)
    {
        gBS->SetMem((void *)(program_header->virtual_address + program_header->file_size), program_header->memory_size - program_header->file_size, 0);
    }
}

