#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <library/common/types.hpp>
#include <kernel/ipc/message.hpp>

#include <stdint.h>

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
            return false; // queue is full
        }

        bool dequeue(process *&proc)
        {
            if (count > 0)
            {
                proc = queue[head];
                head = (head + 1) % QUEUE_SIZE;
                --count;
                return true;
            }
            return false; // queue is empty
        }

      private:
        static constexpr common::word QUEUE_SIZE = 256;
        process *queue[QUEUE_SIZE];
        common::sword head = 0;
        common::sword count = 0;
    };

    constexpr static common::word QUANTUM_MAX = 10;
    constexpr static common::word STACK_SIZE_MAX = 8192;
    constexpr static common::word PROCESS_NAME_MAX = 128;

    enum class process_status : uint16_t
    {
        UNUSED,
        RUNNING,
        READY,
        BLOCKED
    };

    using process_id = common::sword;

    class process
    {
      public:
        process();
        ~process();

        // identifier
        process_id id;
        char name[PROCESS_NAME_MAX];

        // for context-switch
        process_status status;
        common::sword priority;
        common::sword quantum;

        // for priority-scheduling
        process *preview;
        process *next;

        // hardware-context
        void *arch_context;

        uint8_t stack[STACK_SIZE_MAX];
        common::virtual_address stack_pointer;

        common::physical_address page_table;

        // for ipc
        message message_buffer;
        message_queue send_wait_queue;
        process_id receive_from;

        // resolver solves various process-related problems.
        process *resolver;

      private:
    };

}

#endif
