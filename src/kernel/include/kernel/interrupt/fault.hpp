#ifndef A9N_KERNEL_FAULT_HPP
#define A9N_KERNEL_FAULT_HPP

#include <kernel/types.hpp>

namespace a9n::kernel
{
    enum class fault_type : a9n::word
    {
        NONE,                // reserved
        MEMORY,              // e.g., page fault
        INVALID_INSTRUCTION, // e.g., invalid opcode
        INVALID_ARITHMETIC,  // e.g., zero division
        INVALID_KERNEL_CALL, // for emulation
        FATAL,               // unrecoverable
        ARCHITECTURE,
    };

    inline const char *fault_type_to_string(fault_type type)
    {
        switch (type)
        {
            case fault_type::NONE :
                return "NONE";

            case fault_type::MEMORY :
                return "MEMORY";

            case fault_type::INVALID_INSTRUCTION :
                return "INVALID_INSTRUCTION";

            case fault_type::INVALID_ARITHMETIC :
                return "INVALID_ARITHMETIC";

            case fault_type::INVALID_KERNEL_CALL :
                return "INVALID_KERNEL_CALL";

            case fault_type::FATAL :
                return "FATAL";

            case fault_type::ARCHITECTURE :
                return "ARCHITECTURE";

            default :
                return "UNKNOWN";
        }
    }
}

#endif
