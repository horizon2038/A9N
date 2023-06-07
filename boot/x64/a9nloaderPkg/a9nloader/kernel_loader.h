#ifndef LOAD_KERNEL_H
#define LOAD_KERNEL_H

#include <Uefi.h>
#include <stdint.h>
#include <Protocol/SimpleFileSystem.h>

#include "elf.h"
// calculate elf segment.
// load elf segment.
// clear bss to zero.

EFI_STATUS load_kernel(EFI_FILE_PROTOCOL*, uint64_t*);
// uint64_t calculate_file_size(EFI_FILE_PROTOCOL *file, uint64_t *file_size);
EFI_STATUS load_elf_header(EFI_FILE_PROTOCOL*, elf64_header**);
EFI_STATUS load_elf_program_header(EFI_FILE_PROTOCOL*, elf64_header*, elf64_program_header**);
EFI_STATUS load_elf_segment(elf64_header *elf64_header, elf64_program_header*, uint64_t*);
EFI_STATUS locate_elf_segment(elf64_header *header, elf64_program_header *program_header);
void zero_clear(elf64_program_header *program_header);
#endif