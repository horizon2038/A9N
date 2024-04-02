#ifndef ELF_INFO_LOGGER
#define ELF_INFO_LOGGER

#include "elf.h"
#include "stdint.h"

void print_elf_header_info(elf64_header *);
void print_elf_program_header_info(elf64_program_header *);

#endif