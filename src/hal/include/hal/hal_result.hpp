#ifndef A9N_HAL_HAL_RESULT_HPP
#define A9N_HAL_HAL_RESULT_HPP

#include <kernel/types.hpp>
#include <liba9n/result/result.hpp>

namespace a9n::hal
{
    // for kernel and hal boundaries
    enum class hal_error : a9n::word
    {
        PERMISSION_DENIED,
        ILLEGAL_ARGUMENT,
        ILLEGAL_DEVICE,
        NO_SUCH_DEVICE,
        NO_SUCH_ADDRESS,
        TRY_AGAIN,
        UNEXPECTED,
        INIT_FIRST,
        TIMEOUT,
    };

    using hal_result = liba9n::result<void, hal_error>;

    inline constexpr const char *hal_error_to_string(hal_error error)
    {
        switch (error)
        {
            case hal_error::PERMISSION_DENIED :
                return "PERMISSION DENIED";

            case hal_error::ILLEGAL_ARGUMENT :
                return "ILLEGAL ARGUMENT";

            case hal_error::ILLEGAL_DEVICE :
                return "ILLEGAL DEVICE";

            case hal_error::NO_SUCH_DEVICE :
                return "NO SUCH DEVICE";

            case hal_error::NO_SUCH_ADDRESS :
                return "NO SUCH ADDRESS";

            case hal_error::TRY_AGAIN :
                return "TRY AGAIN";

            case hal_error::UNEXPECTED :
                return "UNEXPECTED";

            case hal_error::INIT_FIRST :
                return "INIT FIRST";

            case hal_error::TIMEOUT :
                return "TIMEOUT";

            default :
                return "ERROR NOT FOUND";
        }
    };
}

#endif
