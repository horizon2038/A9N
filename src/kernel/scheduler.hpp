#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <stdint.h>
#include <process.hpp>

namespace kernel
{
    class scheduler
    {
        public:
            scheduler();
            ~scheduler();

            process *schedule_next_process();

        private:

    };
}

#endif
