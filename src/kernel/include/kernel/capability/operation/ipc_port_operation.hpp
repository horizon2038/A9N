#ifndef A9N_KERNEL_IPC_PORT_OPERATION_HPP
#define A9N_KERNEL_IPC_PORT_OPERATION_HPP

#include <kernel/types.hpp>

namespace a9n::kernel
{
    enum class ipc_port_operation : word
    {
        SEND,
        RECEIVE,
        CALL,
        REPLY,
    };

    namespace ipc_port_send_argument
    {
        inline constexpr bool IS_BLOCK      = 0;
        inline constexpr bool MESSAGE_COUNT = 1;
    }

}

#endif
