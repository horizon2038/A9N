#ifndef A9N_HAL_X86_64_IDLE_HPP
#define A9N_HAL_X86_64_IDLE_HPP

namespace a9n::hal::x86_64
{
    // asm
    extern "C" [[noreturn]] void _idle();
}

#endif
