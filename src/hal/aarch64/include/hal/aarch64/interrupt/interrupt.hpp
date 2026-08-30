#ifndef A9N_HAL_AARCH64_INTERRUPT_HPP
#define A9N_HAL_AARCH64_INTERRUPT_HPP

#include <hal/interface/interrupt.hpp>
#include <kernel/interrupt/interrupt.hpp>

namespace a9n::hal::aarch64
{
    inline a9n::kernel::timer_handler          timer_handler {};
    inline a9n::kernel::interrupt_dispatcher   interrupt_dispatcher {};
    inline a9n::kernel::ipi_reschedule_handler ipi_reschedule_handler {};
    inline a9n::kernel::fault_dispatcher       fault_dispatcher {};
    inline a9n::hal::kernel_call_handler       kernel_call_handler {};

    inline constexpr a9n::word IPI_RESCHEDULE_ID     = 0;
    inline constexpr a9n::word IPI_INVALIDATE_TLB_ID = 1;

    extern "C" char aarch64_exception_vectors[];
}

#endif
