#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <kernel/types.hpp>
#include <liba9n/ipc/message.hpp>

// [experimental] A9N::Gloria
#include <kernel/capability/capability_component.hpp>
#include <kernel/ipc/ipc_buffer.hpp>

#include <stdint.h>

namespace a9n::kernel
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
        static constexpr a9n::word QUEUE_SIZE = 256;
        process                   *queue[QUEUE_SIZE];
        a9n::sword                 head  = 0;
        a9n::sword                 count = 0;
    };

    static constexpr a9n::word QUANTUM_MAX      = 10;
    static constexpr a9n::word STACK_SIZE_MAX   = 8192;
    static constexpr a9n::word PROCESS_NAME_MAX = 128;

    enum class process_status : uint16_t
    {
        UNUSED,
        RUNNING,
        READY,
        BLOCKED
    };

    using process_id = a9n::sword;

    class process
    {
      public:
        process();
        ~process();

        // TODO: remove this
        //  identifier
        process_id id;
        char       name[PROCESS_NAME_MAX];

        // for context-switch
        process_status status;
        a9n::sword     priority;
        a9n::sword     quantum;

        // for priority-scheduling
        process *preview;
        process *next;

        // TODO: remove this
        // hardware-context
        void *arch_context;

        uint8_t              stack[STACK_SIZE_MAX];
        a9n::virtual_address stack_pointer;

        a9n::physical_address page_table;

        // for ipc
        ipc::message  message_buffer;
        message_queue send_wait_queue;
        process_id    receive_from;

        // resolver solves various process-related problems.
        process *resolver;

        // [experimental] A9N::Gloria
        struct hardware_context;
        capability_slot *root_slot;
        ipc_buffer      *buffer;
        process         *next_ipc_queue;
        process         *preview_ipc_queue;

      private:
    };

    namespace v2
    {
        // single kernel stack
        class process
        {
          public:
            // for process scheduling
            process_status status { process_status::BLOCKED };
            a9n::sword     priority { 0 };
            a9n::word      quantum { 0 };
            process       *preview;
            process       *next;
            struct hardware_context;

            // for ipc
            ipc_buffer *buffer;
            process    *next_ipc_queue;
            process    *preview_ipc_queue;

            capability_slot *root_slot;
        };
    };

}

#endif
