#include <hal/interface/lock.hpp>

namespace a9n::hal
{
    uint8_t atomic_exchange(volatile uint8_t *address, uint8_t new_value)
    {
        return __atomic_exchange_n(address, new_value, __ATOMIC_ACQ_REL);
    }

    uint32_t atomic_compare_exchange(volatile uint32_t *address, uint32_t expected, uint32_t desired)
    {
        __atomic_compare_exchange_n(address, &expected, desired, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
        return expected;
    }

    uint8_t atomic_load(const volatile uint8_t *address)
    {
        return __atomic_load_n(address, __ATOMIC_ACQUIRE);
    }

    void atomic_store(volatile uint8_t *address, uint8_t new_value)
    {
        __atomic_store_n(address, new_value, __ATOMIC_RELEASE);
    }

    void atomic_store(volatile uint32_t *address, uint32_t new_value)
    {
        __atomic_store_n(address, new_value, __ATOMIC_RELEASE);
    }

    void spin_wait(void)
    {
        asm volatile("yield" ::: "memory");
    }
}
