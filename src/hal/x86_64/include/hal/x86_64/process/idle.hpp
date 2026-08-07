#ifndef A9N_HAL_X86_64_IDLE_HPP
#define A9N_HAL_X86_64_IDLE_HPP

namespace a9n::hal::x86_64
{
    [[noreturn]] inline void idle_loop(void)
    {
        asm volatile("1: jmp 1b" : : : "memory");
        for (;;)
        {
        }
    }
}

#endif
