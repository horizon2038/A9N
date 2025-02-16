#ifndef A9N_KERNEL_INTERRUPT_HPP
#define A9N_KERNEL_INTERRUPT_HPP

#include <kernel/interrupt/fault.hpp>
#include <kernel/types.hpp>

namespace a9n::kernel
{
    using timer_handler        = void (*)(void);
    using interrupt_dispatcher = void (*)(a9n::word irq_number);
    using fault_dispatcher
        = void (*)(fault_type type, a9n::sword arch_fault_code, a9n::virtual_address fault_address);
}

#endif
