#include <hal/interface/lock.hpp>

namespace a9n::hal
{
    uint8_t atomic_exchange(volatile uint8_t *address, uint8_t new_value)
    {
        uint8_t old_value;

        __asm__ __volatile__(
            "lock xchg %0, %1"
            : "=r"(old_value), "+m"(*address)
            : "0"(new_value)
            : "memory"
        );

        return old_value;
    }
}
