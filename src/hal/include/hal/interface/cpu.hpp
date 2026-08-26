#ifndef A9N_HAL_CPU_HPP
#define A9N_HAL_CPU_HPP

#include <hal/hal_result.hpp>

#include <kernel/process/cpu.hpp>

namespace a9n::hal
{
    liba9n::result<a9n::word, hal_error> current_core_number();
    a9n::word                            core_count(void);

    // configure clv per core
    hal_result configure_local_variable(a9n::kernel::cpu_local_variable *target_local_variable);

    liba9n::result<a9n::kernel::cpu_local_variable *, hal_error> current_local_variable();

    hal_result start_other_cores(void);

    void lock(void);
    void unlock(void);
}

#endif
