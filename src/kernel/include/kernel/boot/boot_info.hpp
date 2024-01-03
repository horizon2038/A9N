#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stdint.h>
#include <stdbool.h>

#include <library/common/types.hpp>
#include <kernel/memory/memory_type.hpp>

namespace kernel
{
    constexpr static common::word ARCH_INFO_MAX = 8;

    struct boot_info
    {
        memory_info boot_memory_info;
        common::word arch_info[ARCH_INFO_MAX];
    } __attribute__((packed));
}

#endif
