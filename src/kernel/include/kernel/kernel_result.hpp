#ifndef A9N_KERNEL_KERNEL_RESULT_HPP
#define A9N_KERNEL_KERNEL_RESULT_HPP

#include <hal/hal_result.hpp>
#include <kernel/types.hpp>
#include <liba9n/result/result.hpp>

namespace a9n::kernel
{
    // for kernel and hal boundaries
    enum class kernel_error : a9n::word
    {
        PERMISSION_DENIED,
        ILLEGAL_ARGUMENT,
        ILLEGAL_DEVICE,
        NO_SUCH_DEVICE,
        NO_SUCH_ADDRESS,
        TRY_AGAIN,
        UNEXPECTED,
        INIT_FIRST,
        HAL_ERROR,
    };

    using kernel_result = liba9n::result<void, kernel_error>;

    inline constexpr const char *kernel_error_to_string(kernel_error error)
    {
        switch (error)
        {
            case kernel_error::PERMISSION_DENIED :
                return "PERMISSION DENIED";

            case kernel_error::ILLEGAL_ARGUMENT :
                return "ILLEGAL ARGUMENT";

            case kernel_error::ILLEGAL_DEVICE :
                return "ILLEGAL DEVICE";

            case kernel_error::NO_SUCH_DEVICE :
                return "NO SUCH DEVICE";

            case kernel_error::NO_SUCH_ADDRESS :
                return "NO SUCH ADDRESS";

            case kernel_error::TRY_AGAIN :
                return "TRY AGAIN";

            case kernel_error::UNEXPECTED :
                return "UNEXPECTED";

            case kernel_error::INIT_FIRST :
                return "INIT FIRST";

            case kernel_error::HAL_ERROR :
                return "HAL ERROR";

            default :
                return "ERROR NOT FOUND";
        }
    };

    inline kernel_error convert_hal_to_kernel_error([[maybe_unused]] hal::hal_error e)
    {
        return kernel_error::HAL_ERROR;
    }
}

#endif
