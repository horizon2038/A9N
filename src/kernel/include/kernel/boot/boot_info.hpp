#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stdbool.h>
#include <stdint.h>

#include <kernel/memory/memory_type.hpp>
#include <kernel/types.hpp>

namespace a9n::kernel
{
    static constexpr a9n::word ARCH_INFO_MAX = 8;

    struct init_image_info
    {
        a9n::physical_address loaded_address;
        a9n::word             init_image_size;
        a9n::virtual_address  entry_point_address;
        a9n::virtual_address  init_info_address;
        a9n::virtual_address  init_ipc_buffer_address;
    } __attribute__((packed));

    struct boot_info
    {
        memory_info     boot_memory_info;
        init_image_info boot_init_image_info;
        a9n::word       arch_info[ARCH_INFO_MAX];
    } __attribute__((packed));
}

#endif
