#ifndef A9N_HAL_CPU_HPP
#define A9N_HAL_CPU_HPP

#include <hal/hal_result.hpp>

#include <kernel/process/cpu.hpp>

namespace a9n::hal
{
    liba9n::result<a9n::word, hal_error> current_core_number();

    // configure clv per core
    hal_result configure_local_variable(a9n::kernel::cpu_local_variable *target_local_variable);

    liba9n::result<a9n::kernel::cpu_local_variable *, hal_error> current_local_variable();

}

#endif
