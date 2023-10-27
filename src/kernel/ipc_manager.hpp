#ifndef IPC_HPP
#define IPC_HPP

#include <process.hpp>
#include "message.hpp"

namespace kernel
{
    class ipc_manager
    {
        public:
            ipc_manager() {};
            ~ipc_manager() {};
            bool send(int32_t receiver_process_id, message *msg);
            bool receive(int32_t sender_process_id, message *msg);

            // void notify();

            // impl IPC-fastpath
    };
}

#endif
