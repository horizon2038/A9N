#ifndef A9N_KERNEL_CAPABILITY_RESULT_HPP
#define A9N_KERNEL_CAPABILITY_RESULT_HPP

#include <kernel/types.hpp>
#include <liba9n/result/result.hpp>

namespace a9n::kernel
{
    enum class capability_error : a9n::word
    {
        ILLEGAL_OPERATION,
        INVALID_DESCRIPTOR,
        INVALID_DEPTH,
        INVALID_ARGUMENT,

        DEBUG_UNIMPLEMENTED,
    };

    using capability_result = liba9n::result<void, capability_error>;
}

#endif
