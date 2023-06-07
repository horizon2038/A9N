#include "elf_info_logger.h"

#include <Uefi.h>
#include <Library/UefiLib.h>

void print_elf_header_info(elf64_header *header)
{
    Print(L"=====ELF_HEADER=====\r\n");
    Print(L"\tmagic: 0x%x %c%c%c\n", header->identifier[0], header->identifier[1], header->identifier[2], header->identifier[3]);
    Print(L"\ttype: %llu\n", header->type);
    Print(L"\tmachine: %llu\n", header->machine);
    Print(L"\tentry_point_address: 0x%04lx\n", header->entry_point_address);
    Print(L"\tprogram_header_offset: 0x%04lx\n", header->program_header_offset);
    Print(L"\tprogram_header_number: %d\r\n", header->program_header_number);
}

void print_elf_program_header_info(elf64_program_header *program_header)
{
    Print(L"=====ELF_PROGRAM_HEADER=====\r\n");
    Print(L"type: %u\n", program_header->type);
    Print(L"flags: %llu\n", program_header->flags);
    Print(L"offset: 0x%04llx\n", program_header->offset);
    Print(L"virtual_address: 0x%04llx\n", program_header->virtual_address);
    Print(L"physical_address: 0x%04llx\n", program_header->physical_address);
    Print(L"file_size: 0x%04llx\n", program_header->file_size);
    Print(L"memory_size: 0x%04llx\n", program_header->memory_size);
    Print(L"alignment: 0x%llx\n", program_header->alignment);
}