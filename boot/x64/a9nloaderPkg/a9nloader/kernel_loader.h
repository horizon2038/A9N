#ifndef LOAD_KERNEL_H
#define LOAD_KERNEL_H

#include <Uefi.h>
#include <stdint.h>
#include <Protocol/SimpleFileSystem.h>

#include "elf.h"
// calculate elf segment.
// load elf segment.
// clear bss to zero.

EFI_STATUS load_kernel(EFI_FILE_PROTOCOL *kernel, uint64_t offset);
uint64_t calculate_file_size(EFI_FILE_PROTOCOL *file, uint64_t *file_size);
EFI_STATUS calculate_elf_segment(elf64_header *elf64_header, EFI_PHYSICAL_ADDRESS image_base);
EFI_STATUS load_elf_segment(elf64_header *header, elf64_program_header *program_header, EFI_PHYSICAL_ADDRESS segment_start);
EFI_STATUS zero_clear(elf64_program_header *program_header, EFI_PHYSICAL_ADDRESS segment_start, uint16_t count);

#endif