#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <stdint.h>
#include <common.hpp>
#include <message.hpp>

namespace kernel
{
    class message_queue
    {
        public:
            bool enqueue(const message& msg)
            {
                if (count < QUEUE_SIZE)
                {
                    queue[(head + count) % QUEUE_SIZE] = msg;
                    ++count;
                    return true;
                }
                return false;  // Queue is full
            }

            bool dequeue(message& msg)
            {
                if (count > 0)
                {
                    msg = queue[head];
                    head = (head + 1) % QUEUE_SIZE;
                    --count;
                    return true;
                }
                return false;  // Queue is empty
            }

        private:
            static constexpr int QUEUE_SIZE = 256;
            message queue[QUEUE_SIZE];
            int head = 0;
            int count = 0;
    };

    inline constexpr uint32_t STACK_SIZE_MAX = 8192;
    inline constexpr uint16_t PROCESS_NAME_MAX = 128;

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

            // hardware-context
            uint8_t stack[STACK_SIZE_MAX];
            virtual_address stack_pointer;
            physical_address page_table;

            // for ipc
            message_queue send_wait_queue;

        private:

    };

}

#endif

