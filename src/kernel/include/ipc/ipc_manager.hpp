#ifndef IPC_HPP
#define IPC_HPP

#include <process/process.hpp>
#include <ipc/message.hpp>

namespace kernel
{
    static constexpr int32_t ANY_PROCESS = -1;
    class ipc_manager
    {
        public:
            ipc_manager() {};
            ~ipc_manager() {};
            void send(process_id receiver_process_id, message *msg);
            void receive(process_id sender_process_id, message *msg);

            // void notify();
    };
}

#endif
