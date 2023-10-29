#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <stdint.h>
#include <process.hpp>

namespace kernel
{
    struct process_node
    {
        process *process;
        process_node *next;
    };

    constexpr static int32_t PRIORITY_MAX = 32;

    class scheduler
    {

        public:
            scheduler(process *process_list);
            ~scheduler();

            process *schedule_next_process();
            process *schedule_next_process(process *priority_groups[], int32_t &priority);

        private:
            process *_process_list; // process array pointer
            process *_current_process;
            uint64_t current_process_index;

            void move_to_end(process *target_process,  process *&head_process);
            int32_t update_highest_priority(process *priority_groups[]); 

    };
}

#endif
