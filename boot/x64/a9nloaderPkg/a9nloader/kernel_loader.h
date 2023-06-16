#ifndef LOAD_KERNEL_H
#define LOAD_KERNEL_H

#include <Uefi.h>
#include <stdint.h>
#include <Protocol/SimpleFileSystem.h>

#include "elf.h"

EFI_STATUS load_kernel(EFI_FILE_PROTOCOL*, uint64_t*);

#endif