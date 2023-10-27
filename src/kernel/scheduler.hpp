#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <stdint.h>
#include <process.hpp>

namespace kernel
{
    class scheduler
    {
        public:
            scheduler(process *process_list);
            ~scheduler();

            process *schedule_next_process();

        private:
            process *_process_list; // process array pointer
            process *_current_process;
            uint64_t current_process_index;

    };
}

#endif
