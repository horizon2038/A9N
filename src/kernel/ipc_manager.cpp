#include "ipc_manager.hpp"
#include "kernel.hpp"
#include "process.hpp"

#include <library/string.hpp>
#include <library/logger.hpp>

namespace kernel
{
    void ipc_manager::send(int32_t receiver_process_id, message *msg)
    {
        process* receiver_process = kernel_object::process_manager->search_process_from_id(receiver_process_id);
        utility::logger::printk("ipc_send : %s -> %s\n", kernel_object::process_manager->current_process->name, receiver_process->name);
        utility::logger::printk("send message | type : %d | data : %s\n", msg->type, reinterpret_cast<const char*>(msg->data));

        process* current_process = kernel_object::process_manager->current_process;
        std::memcpy(&current_process->message_buffer, msg, sizeof(message));

        if (receiver_process == nullptr)
        {
            return;
        }

        if (receiver_process->id == current_process->id)
        {
            return;
        }

        bool is_ready = receiver_process->status == process_status::BLOCKED && 
            (receiver_process->receive_from == ANY_PROCESS || 
             receiver_process->receive_from == current_process->id);

        if (is_ready)
        {
            current_process->status = process_status::BLOCKED;
            receiver_process->send_wait_queue.enqueue(current_process);
            kernel_object::process_manager->switch_context();
        }
        else
        {
            std::memcpy(&receiver_process->message_buffer, &current_process->message_buffer, sizeof(message));

            receiver_process->status = process_status::READY;
            return;
        }
    }

    void ipc_manager::receive(int32_t source_process_id, message *msg)
    {
        process* current_process = kernel_object::process_manager->current_process;
        process* sender_process = nullptr;

        current_process->receive_from = source_process_id;

        bool dequeued = current_process->send_wait_queue.dequeue(sender_process);

        if (!dequeued || (source_process_id != ANY_PROCESS && sender_process->id != source_process_id))
        {
            current_process->status = process_status::BLOCKED;
            kernel_object::process_manager->switch_context();
        }
        else
        {
            std::memcpy(msg, &sender_process->message_buffer, sizeof(message));

            sender_process->status = process_status::READY;
            return;
        }
    }
}

