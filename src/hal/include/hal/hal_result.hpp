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
    };

    using hal_result = liba9n::result<void, hal_error>;
}

#endif
