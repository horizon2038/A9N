#include "ipc_manager.hpp"
#include "kernel.hpp"

namespace kernel
{
    bool ipc_manager::send(int32_t receiver_process_id, const message& msg)
    {
        process* receiver_process = kernel_object::process_manager->search_process_from_id(receiver_process_id);

        if (receiver_process == nullptr)
        {
            return false;
        }

        if (receiver_process->status == process_status::BLOCKED)
        {
            receiver_process->message_buffer = msg;
            receiver_process->status = process_status::READY;
            kernel_object::process_manager->switch_context();
        }
        else
        {
            if (!kernel_object::process_manager->current_process->send_wait_queue.enqueue(receiver_process))
            {
                return false;
            }
        }
        return true;
    }

    bool ipc_manager::receive(message& msg)
    {
        process* sender_process;

        if (kernel_object::process_manager->current_process->send_wait_queue.dequeue(sender_process))
        {
            msg = sender_process->message_buffer;
            return true;
        }

        kernel_object::process_manager->current_process->status = process_status::BLOCKED;
        kernel_object::process_manager->switch_context();
        return false;
    }
}

