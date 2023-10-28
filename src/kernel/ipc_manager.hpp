#ifndef IPC_HPP
#define IPC_HPP

#include <process.hpp>
#include "message.hpp"

namespace kernel
{
    static constexpr int32_t ANY_PROCESS = -1;
    class ipc_manager
    {
        public:
            ipc_manager() {};
            ~ipc_manager() {};
            void send(int32_t receiver_process_id, message *msg);
            void receive(int32_t sender_process_id, message *msg);

            // void notify();

            // impl IPC-fastpath
    };
}

#endif
