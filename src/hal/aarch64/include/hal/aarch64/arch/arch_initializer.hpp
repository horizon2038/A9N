#ifndef A9N_HAL_AARCH64_ARCH_INITIALIZER_HPP
#define A9N_HAL_AARCH64_ARCH_INITIALIZER_HPP

#include <hal/interface/arch_initializer.hpp>

namespace a9n::hal::aarch64
{
    hal_result init_architecture_impl(a9n::word arch_info[]);
}

#endif
