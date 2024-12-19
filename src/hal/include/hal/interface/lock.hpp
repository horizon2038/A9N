#ifndef A9N_HAL_LOCK_HPP
#define A9N_HAL_LOCK_HPP

#include <hal/hal_result.hpp>

namespace a9n::hal
{
    uint8_t atomic_exchange(volatile uint8_t *address, uint8_t new_value);
}

#endif
