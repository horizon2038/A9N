#ifndef A9N_HAL_LOCK_HPP
#define A9N_HAL_LOCK_HPP

#include <hal/hal_result.hpp>

namespace a9n::hal
{
    uint8_t atomic_exchange(volatile uint8_t *address, uint8_t new_value);
    uint32_t atomic_compare_exchange(
        volatile uint32_t *address,
        uint32_t expected,
        uint32_t desired
    );
    uint8_t atomic_load(const volatile uint8_t *address);
    void    atomic_store(volatile uint8_t *address, uint8_t new_value);
    void    atomic_store(volatile uint32_t *address, uint32_t new_value);
}

#endif
