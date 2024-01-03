#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stdint.h>
#include <stdbool.h>

#include <library/common/types.hpp>
#include <kernel/memory/memory_type.hpp>

namespace kernel
{
    struct boot_info
    {
        memory_info boot_memory_info;
        common::word arch_info[8];
    } __attribute__((packed));
}

#endif
