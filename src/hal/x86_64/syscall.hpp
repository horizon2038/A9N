#ifndef x86_64_SYSCALL_HPP
#define x86_64_SYSCALL_HPP

#include <stdint.h>

namespace hal::x86_64
{
    constexpr static uint32_t MSR_STAR = 0xc0000082;
    constexpr static uint32_t MSR_LSTAR = 0xc0000083;
    constexpr static uint32_t MSR_CSTAR = 0xc0000084;
    constexpr static uint32_t MSR_SFMASK = 0xc0000084;
}

#endif
