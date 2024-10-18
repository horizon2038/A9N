#ifndef A9N_BOOT_A9N_LOADER_INIT_BOOT_INFO_H
#define A9N_BOOT_A9N_LOADER_INIT_BOOT_INFO_H

#include <stdint.h>

typedef struct
{
    // physical address of load destination
    uintmax_t loaded_address;
    // init_image physical size
    uintmax_t init_image_size;

    // virtual entry point address
    uintmax_t entry_point_address;

    // virtual init info address
    uintmax_t init_info_address;
    uintmax_t init_ipc_buffer_address;
} init_image_info;

#endif
