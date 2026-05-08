#ifndef x86_64_SYSCALL_HPP
#define x86_64_SYSCALL_HPP

#include <hal/interface/interrupt.hpp>
#include <stdint.h>

namespace a9n::hal::x86_64
{
    hal_result init_syscall();

    inline kernel_call_handler kernel_call_handler {};
}

#endif
