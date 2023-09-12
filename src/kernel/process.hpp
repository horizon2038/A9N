#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <stdint.h>

namespace kernel
{
    namespace
    {
        inline constexpr uint32_t STACK_SIZE_MAX = 8192;
        using memory_address = uint32_t*;
    }

    class process
    {
        public:
            process();
            ~process();

            int32_t pid;
            uint32_t status;
            uint32_t priority;
            uint8_t stack[STACK_SIZE_MAX];
            uint32_t *stack_pointer;

        private:

    };
}

#endif

