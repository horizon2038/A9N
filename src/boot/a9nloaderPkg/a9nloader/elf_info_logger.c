#include "elf_info_logger.h"

#include <Library/UefiLib.h>
#include <Uefi.h>

void print_elf_header_info(elf64_header *header)
{
    Print(L"[ INFO ] elf_header\r\n");
    Print(
        L"magic: 0x%x %c%c%c\n",
        header->identifier[0],
        header->identifier[1],
        header->identifier[2],
        header->identifier[3]
    );
    Print(L"type: %llu\n", header->type);
    Print(L"machine: %llu\n", header->machine);
    Print(L"entry_point_address: 0x%04lx\n", header->entry_point_address);
    Print(L"program_header_offset: 0x%04lx\n", header->program_header_offset);
    Print(L"program_header_number: %d\r\n", header->program_header_number);
    Print(L"\r\n");
}

void print_elf_program_header_info(elf64_program_header *program_header)
{
    Print(L"[ INFO ] elf_program_header\r\n");
    Print(L"program_header_address: 0x%04llx\n", (uint64_t)program_header);
    Print(L"type: %u\n", program_header->type);
    Print(L"flags: %llu\n", program_header->flags);
    Print(L"offset: 0x%04llx\n", program_header->offset);
    Print(L"virtual_address: 0x%04llx\n", program_header->virtual_address);
    Print(L"physical_address: 0x%04llx\n", program_header->physical_address);
    Print(L"file_size: 0x%04llx\n", program_header->file_size);
    Print(L"memory_size: 0x%04llx\n", program_header->memory_size);
    Print(L"alignment: 0x%llx\n", program_header->alignment);
    Print(L"\r\n");
}
