#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <stdint.h>

namespace kernel
{
    namespace
    {
        inline constexpr uint32_t STACK_SIZE_MAX = 8192;
    }

    class process
    {
        public:
            process();
            ~process();

            int32_t id;
            uint32_t status;
            uint32_t priority;
            uint8_t stack[STACK_SIZE_MAX];
            uint64_t *stack_pointer;
            uint64_t page_table;

        private:

    };

}

#endif

