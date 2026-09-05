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

    uint32_t atomic_compare_exchange(volatile uint32_t *address, uint32_t expected, uint32_t desired)
    {
        asm volatile("lock cmpxchgl %2, %1"
                     : "+a"(expected), "+m"(*address)
                     : "r"(desired)
                     : "memory", "cc");
        return expected;
    }

    uint8_t atomic_load(const volatile uint8_t *address)
    {
        uint8_t value;
        asm volatile("movb %1, %0" : "=q"(value) : "m"(*address) : "memory");
        return value;
    }

    void atomic_store(volatile uint8_t *address, uint8_t new_value)
    {
        asm volatile("movb %1, %0" : "=m"(*address) : "q"(new_value) : "memory");
    }

    void atomic_store(volatile uint32_t *address, uint32_t new_value)
    {
        asm volatile("movl %1, %0" : "=m"(*address) : "r"(new_value) : "memory");
    }

    void spin_wait(void)
    {
        asm volatile("pause" ::: "memory");
    }
}
