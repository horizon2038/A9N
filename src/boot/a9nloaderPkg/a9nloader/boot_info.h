#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stdbool.h>
#include <stdint.h>

#include "memory_type.h"

typedef struct
{
    memory_info boot_memory_info;
    uintmax_t   arch_info[8];
} boot_info;

#endif
