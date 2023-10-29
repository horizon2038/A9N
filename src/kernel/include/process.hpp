#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <stdint.h>
#include <common.hpp>
#include <message.hpp>

namespace kernel
{
    class process;

    class message_queue
    {
        public:
            bool enqueue(process *proc)
            {
                if (count < QUEUE_SIZE)
                {
                    queue[(head + count) % QUEUE_SIZE] = proc;
                    ++count;
                    return true;
                }
                return false;  // queue is full
            }

            bool dequeue(process*& proc)
            {
                if (count > 0)
                {
                    proc = queue[head];
                    head = (head + 1) % QUEUE_SIZE;
                    --count;
                    return true;
                }
                return false;  // queue is empty
            }

        private:
            static constexpr int QUEUE_SIZE = 256;
            process* queue[QUEUE_SIZE];
            int head = 0;
            int count = 0;
    };

    constexpr static uint32_t QUANTUM_MAX = 20;
    constexpr static uint32_t STACK_SIZE_MAX = 8192;
    constexpr static uint16_t PROCESS_NAME_MAX = 128;

    enum class process_status : uint16_t
    {
        UNUSED,
        RUNNING,
        READY,
        BLOCKED
    };

    class process
    {
        public:
            process();
            ~process();

            // identifier
            int32_t id;
            char name[PROCESS_NAME_MAX];

            // for context-switch
            process_status status;
            uint32_t priority;
            uint32_t quantum;

            // for priority-scheduling
            process *preview;
            process *next;

            // hardware-context
            uint8_t stack[STACK_SIZE_MAX];
            virtual_address stack_pointer;
            physical_address page_table;

            // for ipc
            message message_buffer;
            message_queue send_wait_queue;
            int32_t receive_from;

        private:

    };

}

#endif

