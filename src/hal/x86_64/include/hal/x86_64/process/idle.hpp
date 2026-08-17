#ifndef A9N_HAL_X86_64_IDLE_HPP
#define A9N_HAL_X86_64_IDLE_HPP

namespace a9n::hal::x86_64
{
    [[noreturn]] inline void idle_loop(void)
    {
        // Interrupt recognition is inhibited until after the instruction following STI. Keeping
        // STI and HLT adjacent therefore avoids losing a wake-up between enabling interrupts and
        // halting the processor.
        asm volatile(
            // Discard the current kernel call chain and park on this CPU's Single Kernel Stack.
            // IDLE is noreturn, so no caller frame needs to be preserved.
            "movq %%gs:0x00, %%rsp\n\t"
            "1:\n\t"
            "sti\n\t"
            "hlt\n\t"
            "jmp 1b"
            :
            :
            : "memory"
        );
        for (;;)
        {
        }
    }
}

#endif
