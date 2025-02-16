#ifndef A9N_KERNEL_CAPABILITY_TYPE_HPP
#define A9N_KERNEL_CAPABILITY_TYPE_HPP

#include <kernel/types.hpp>

namespace a9n::kernel
{
    enum class capability_type : uint8_t
    {
        // reserved
        NONE,
        DEBUG,

        // composite
        NODE,

        // memory
        GENERIC,
        ADDRESS_SPACE, // alias of root page table
        PAGE_TABLE,
        FRAME,

        // process
        PROCESS_CONTROL_BLOCK,

        // communication
        IPC_PORT,
        NOTIFICATION_PORT,

        // driver
        INTERRUPT_REGION,
        INTERRUPT_PORT,
        IO_PORT,

        // virtualization
        VIRTUAL_CPU,
        VIRTUAL_ADDRESS_SPACE, // alias of root virtual page table
        VIRTUAL_PAGE_TABLE,
    };
}

#endif
