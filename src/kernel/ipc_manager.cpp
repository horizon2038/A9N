#include "ipc_manager.hpp"
#include "kernel.hpp"

namespace kernel
{
    bool ipc_manager::send(int32_t receiver_process_id, message *msg)
    {
        process *receiver_process = kernel_object::process_manager->search_process_from_id(receiver_process_id);
        if (receiver_process == nullptr)
        {
            return false;
        }
        if (receiver_process_id == kernel_object::process_manager->current_process->id)
        {
            return false;
        }

        /* COPY MESSAGE */
    }
}

