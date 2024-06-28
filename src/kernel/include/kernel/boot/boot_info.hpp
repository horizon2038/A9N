#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stdint.h>
#include <stdbool.h>

#include <kernel/types.hpp>
#include <kernel/memory/memory_type.hpp>

namespace a9n::kernel
{
    constexpr static a9n::word ARCH_INFO_MAX = 8;

    struct boot_info
    {
        memory_info boot_memory_info;
        a9n::word   arch_info[ARCH_INFO_MAX];
    } __attribute__((packed));
}

#endif
