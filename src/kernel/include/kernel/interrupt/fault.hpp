#ifndef A9N_KERNEL_FAULT_HPP
#define A9N_KERNEL_FAULT_HPP

#include <kernel/types.hpp>

namespace a9n::kernel
{
    enum class fault_type : a9n::word
    {
        NONE,   // reserved
        MEMORY, // e.g., page fault
        MEMORY_INSTRUCTION_FETCH,
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

            case fault_type::MEMORY_INSTRUCTION_FETCH :
                return "MEMORY_INSTRUCTION_FETCH";

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

    namespace fault_index
    {
        enum kernel_to_user : a9n::word
        {
            IS_SUCCESS,   // MR0 - descriptor
            ERROR_CODE,   // MR1 - type
            MESSAGE_INFO, // MR2 - info
            IDENTIFIER,   // MR3 - id
            FAULT_REASON, // MR4 - tag
            RESERVED = FAULT_REASON,
        };
    }

    // memory and instruction fetch
    namespace fault_memory_index
    {
        // kernel -> user
        enum kernel_to_user : a9n::word
        {
            RESERVED = fault_index::RESERVED,
            FAULT_PROGRAM_COUNTER,   // MR5
            FAULT_ADDRESS,           // MR6
            ARCHITECTURE_FAULT_CODE, // MR7
            MESSAGE_LENGTH = 5,
        };

        // user -> kernel
        enum user_to_kernel : a9n::word
        {
        };
    };

    namespace fault_invalid_instruction_index
    {
        enum kernel_to_user : a9n::word
        {
            RESERVED = fault_index::RESERVED,
            FAULT_PROGRAM_COUNTER,
            ARCHITECTURE_FAULT_CODE,
            MESSAGE_LENGTH,
        };
    };

    namespace fault_invalid_arithmetic_index
    {
        enum kernel_to_user : a9n::word
        {
            RESERVED = fault_index::RESERVED,
            FAULT_PROGRAM_COUNTER,
            ARCHITECTURE_FAULT_CODE,
            MESSAGE_LENGTH,
        };
    };

    namespace fault_invalid_kernel_call_index
    {
        enum kernel_to_user : a9n::word
        {
            RESERVED = fault_index::RESERVED,
            FAULT_PROGRAM_COUNTER,
            KERNEL_CALL_NUMBER,
            MESSAGE_LENGTH,
        };
    };

    namespace fault_architecture_index
    {
        enum kernel_to_user : a9n::word
        {
            RESERVED = fault_index::RESERVED,
            FAULT_PROGRAM_COUNTER,
            ARCHITECTURE_FAULT_CODE,
            MESSAGE_LENGTH,
        };
    }
}

#endif
