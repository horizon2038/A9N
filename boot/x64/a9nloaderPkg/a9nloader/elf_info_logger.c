#include "elf_info_logger.h"

#include <Uefi.h>
#include <Library/UefiLib.h>

void print_elf_info(elf64_header *header)
{
    Print(L"magic: 0x%x %c%c%c\n", header->identifier[0], header->identifier[1], header->identifier[2], header->identifier[3]);
    Print(L"type: %llu\n", header->type);
    Print(L"machine: %llu\n", header->machine);
    Print(L"entry_point_address: 0x%04lx\n", header->entry_point_address);
    Print(L"program_header_offset: 0x%04lx\n", header->program_header_offset);
    // Print(L"section_header_offset: 0x%04lx\n", header->section_header_offset);
    Print(L"program_header_number: %d\r\n", header->program_header_number);
}