#ifndef A9N_KERNEL_CAPABILITY_TYPE_HPP
#define A9N_KERNEL_CAPABILITY_TYPE_HPP

#include <kernel/types.hpp>

namespace a9n::kernel
{
    enum class capability_type : uint16_t
    {
        // reserved
        NONE,
        DEBUG,

        // composite
        NODE,

        // memory
        GENERIC,
        PAGE_TABLE,
        FRAME,

        // process
        DOMAIN,
        PROCESS_CONTROL_BLOCK,

        // communication
        IPC_PORT,
        NOTIFICATION_PORT,
        INTERRUPT,

        // virtualization
        VIRTUAL_CPU,
    };
}

#endif
