#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stdbool.h>
#include <stdint.h>

#include <kernel/memory/memory_type.hpp>
#include <kernel/types.hpp>

namespace a9n::kernel
{
    static constexpr a9n::word ARCH_INFO_MAX = 8;

    struct boot_info
    {
        memory_info boot_memory_info;
        a9n::word   arch_info[ARCH_INFO_MAX];
    } __attribute__((packed));
}

#endif
