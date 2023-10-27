#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <stdint.h>
#include <common.hpp>

namespace kernel
{

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

        private:

    };

}

#endif

