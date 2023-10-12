#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include <stddef.h>
#include <stdint.h>

#include "boot_info.h"

class memory_manager
{
    public:
        memory_manager();
        void init();
        void *allocate_physical_memory(size_t size);
};

#endif
