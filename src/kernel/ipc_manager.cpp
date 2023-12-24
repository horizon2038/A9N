#include <kernel/ipc/ipc_manager.hpp>

#include "kernel.hpp"
#include <process/process.hpp>

#include <library/string.hpp>
#include <library/logger.hpp>

namespace kernel
{
    void ipc_manager::send(process_id receiver_process_id, message *msg)
    {
        process *receiver_process
            = kernel_object::process_manager->search_process_from_id(
                receiver_process_id
            );
        utility::logger::printk(
            "ipc_send : %s -> %s\n",
            kernel_object::process_manager->current_process->name,
            receiver_process->name
        );
        utility::logger::printk(
            "message | type : %d | data : \e[32m%s\e[0m\n",
            msg->type,
            reinterpret_cast<const char *>(msg->data)
        );

        process *current_process
            = kernel_object::process_manager->current_process;
        std::memcpy(&current_process->message_buffer, msg, sizeof(message));

        if (receiver_process == nullptr || receiver_process == current_process)
        {
            // Handle error: Invalid receiver or trying to send message to
            // itself
            return;
        }

        bool is_ready
            = receiver_process->status == process_status::BLOCKED
           && (receiver_process->receive_from == ANY_PROCESS
               || receiver_process->receive_from == current_process->id);

        if (!is_ready)
        {
            std::memset(msg, 0, sizeof(message));
            current_process->status = process_status::BLOCKED;
            receiver_process->send_wait_queue.enqueue(current_process);
            kernel_object::process_manager->switch_context();
        }
        else
        {
            std::memcpy(
                &receiver_process->message_buffer,
                &current_process->message_buffer,
                sizeof(message)
            );

            receiver_process->status = process_status::READY;

            // kernel_object::process_manager->switch_context();
        }
    }

    void ipc_manager::receive(process_id source_process_id, message *msg)
    {
        process *current_process
            = kernel_object::process_manager->current_process;
        process *sender_process = nullptr;

        current_process->receive_from = source_process_id;

        // TODO: fix flag
        bool dequeued
            = current_process->send_wait_queue.dequeue(sender_process);

        if (!dequeued
            || (source_process_id != ANY_PROCESS
                && sender_process->id != source_process_id))
        {
            std::memset(msg, 0, sizeof(message));
            current_process->status = process_status::BLOCKED;
            kernel_object::process_manager->switch_context();
        }
        else
        {
            /*
            utility::logger::printk
            (
                "ipc_receive : %d %s <- %d %s\n",
                kernel_object::process_manager->current_process->id,
                kernel_object::process_manager->current_process->name,
                sender_process->id,
                sender_process->name
            );
            */
            std::memcpy(msg, &sender_process->message_buffer, sizeof(message));

            sender_process->status = process_status::READY;
        }
    }
}
