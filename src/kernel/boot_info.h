#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stdint.h>
#include <stdbool.h>

#include "memory_type.h"

typedef struct
{
    memory_info boot_memory_info;
} boot_info;

#endif
