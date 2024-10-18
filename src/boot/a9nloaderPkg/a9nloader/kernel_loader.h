#ifndef LOAD_KERNEL_H
#define LOAD_KERNEL_H

#include "stdint.h"
#include <Protocol/SimpleFileSystem.h>
#include <Uefi.h>

#include "elf.h"

EFI_STATUS load_kernel(EFI_FILE_PROTOCOL *kernel_file, uint64_t *kernel_entry_point);

EFI_STATUS load_init_server(
    EFI_FILE_PROTOCOL *init_server_file,
    uint64_t          *init_server_loaded_address,
    uint64_t          *init_server_memory_size,
    uint64_t          *init_server_entry_point,
    uint64_t          *init_server_info_address,
    uint64_t          *init_server_ipc_buffer_address
);

#endif
