#ifndef SYSCALL_HPP
#define SYSCALL_HPP

#include <liba9n/common/types.hpp>
#include <stdint.h>

namespace a9n::kernel
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
        void handle_system_call(
            uint16_t     system_call_number,
            a9n::word arg_1,
            a9n::word arg_2,
            a9n::word arg_3,
            a9n::word arg_4,
            a9n::word arg_5
        );

      private:
        void init_system_call_table();
    };
}

#endif
