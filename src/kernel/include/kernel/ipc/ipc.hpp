#ifndef A9N_KERNEL_IPC_HPP
#define A9N_KERNEL_IPC_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/process/process.hpp>

namespace a9n::kernel
{
    namespace ipc_argument
    {
        inline constexpr a9n::word FLAGS = 0;
    }

    struct ipc_port
    {
        v2::process *next;
        v2::process *preview;
    };

    inline void send_ipc(
        ipc_buffer      *target_buffer,
        v2::process     *this_process,
        capability_slot *this_slot
    )
    {
    }

    inline void receive_ipc(
        ipc_buffer      *target_buffer,
        v2::process     *this_process,
        capability_slot *this_slot
    )
    {
    }
}

#endif
