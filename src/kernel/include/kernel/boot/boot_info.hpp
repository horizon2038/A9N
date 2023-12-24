#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stdint.h>
#include <stdbool.h>

#include <kernel/memory/memory_type.hpp>

namespace kernel
{
    struct boot_info
    {
        memory_info boot_memory_info;
    };
}

#endif
