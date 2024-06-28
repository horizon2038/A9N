#ifndef SYSCALL_HPP
#define SYSCALL_HPP

#include <library/common/types.hpp>
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
        void handle_system_call(
            uint16_t     system_call_number,
            common::word arg_1,
            common::word arg_2,
            common::word arg_3,
            common::word arg_4,
            common::word arg_5
        );

      private:
        void init_system_call_table();
    };
}

#endif
