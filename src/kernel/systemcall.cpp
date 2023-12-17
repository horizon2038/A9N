#include "systemcall.hpp"

#include <stdint.h>

#include "kernel.hpp"

namespace kernel
{
    void system_call_manager::init()
    {
    }

    void system_call_manager::init_system_call_table()
    {
    }

    void system_call_manager::handle_system_call(uint16_t system_call_number, uint64_t arg_1, uint64_t arg_2, uint64_t arg_3, uint64_t arg_4, uint64_t arg_5)
    {
        switch(system_call_number)
        {
            case SYSTEMCALL_TYPE::IPC :
                break;

            case SYSTEMCALL_TYPE::YIELD :
                break;

        }
    }
}
