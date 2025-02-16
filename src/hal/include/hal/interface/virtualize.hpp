#ifndef A9N_HAL_VIRTUALIZE_HPP
#define A9N_HAL_VIRTUALIZE_HPP

#include <hal/hal_result.hpp>
#include <kernel/virtualization/virtual_cpu.hpp>
#include <liba9n/common/not_null.hpp>

namespace a9n::hal
{
    hal_result try_init_virtual_cpu(a9n::kernel::virtual_cpu &cpu);
}

#endif
