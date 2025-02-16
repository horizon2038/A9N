#ifndef A9N_KERNEL_CAPABILITY_RESULT_HPP
#define A9N_KERNEL_CAPABILITY_RESULT_HPP

#include <kernel/kernel_result.hpp>
#include <kernel/types.hpp>
#include <liba9n/result/result.hpp>

namespace a9n::kernel
{
    enum class capability_error : a9n::word
    {
        ILLEGAL_OPERATION,
        PERMISSION_DENIED,
        INVALID_DESCRIPTOR,
        INVALID_DEPTH,
        INVALID_ARGUMENT,
        FATAL,

        DEBUG_UNIMPLEMENTED,
    };

    using capability_result = liba9n::result<void, capability_error>;

    inline constexpr const char *capability_error_to_string(capability_error error)
    {
        switch (error)
        {
            case capability_error::ILLEGAL_OPERATION :
                return "ILLEGAL OPERATION";

            case capability_error::PERMISSION_DENIED :
                return "PERMISSION DENIED";

            case capability_error::INVALID_DESCRIPTOR :
                return "INVALID DESCRIPTOR";

            case capability_error::INVALID_DEPTH :
                return "INVALID DEPTH";

            case capability_error::INVALID_ARGUMENT :
                return "INVALID ARGUMENT";

            case capability_error::FATAL :
                return "FATAL";

            case capability_error::DEBUG_UNIMPLEMENTED :
                return "DEBUG_UNIMPLEMENTED";

            default :
                return "ERROR NOT FOUND";
        }
    };

    inline capability_error convert_kernel_to_capability_error([[maybe_unused]] kernel_error e)
    {
        return capability_error::FATAL;
    }

}

#endif
