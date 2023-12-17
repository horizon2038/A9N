#ifndef SYSCALL_HPP
#define SYSCALL_HPP

#include <stdint.h>

namespace kernel
{
    namespace SYSTEMCALL_TYPE
    {
        // IPC section
        constexpr static uint16_t IPC = 0;

        // other section
        constexpr static uint16_t YIELD = 1;
    }

    class system_call_manager
    {
        public:
            void init();

            // hardware-independent systemcall handler
            void handle_system_call(uint16_t system_call_number, uint64_t arg_1, uint64_t arg_2, uint64_t arg_3, uint64_t arg_4, uint64_t arg_5);

        private:
            void init_system_call_table();
    };
}

#endif
