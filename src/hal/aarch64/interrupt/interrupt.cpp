#include <hal/interface/interrupt.hpp>

#include <hal/aarch64/arch/arch_context.hpp>
#include <hal/aarch64/arch/cpu.hpp>
#include <hal/aarch64/interrupt/interrupt.hpp>
#include <hal/aarch64/platform.hpp>

#include <kernel/interrupt/fault.hpp>
#include <kernel/process/cpu.hpp>
#include <kernel/utility/logger.hpp>

namespace a9n::hal::aarch64
{
    namespace
    {
        inline a9n::word active_irq = 1023;

        [[noreturn]] void fatal_exception(const exception_frame &frame)
        {
            a9n::kernel::utility::logger::printh(
                "Fatal AArch64 exception: ESR_EL1=0x%016llx FAR_EL1=0x%016llx "
                "ELR_EL1=0x%016llx\n",
                frame.esr_el1,
                frame.far_el1,
                frame.registers[register_index::ELR_EL1]
            );
            asm volatile("msr daifset, #0xf" ::: "memory");
            for (;;)
            {
                asm volatile("wfe");
            }
        }

        a9n::kernel::hardware_context *save_current_context(const exception_frame &frame)
        {
            auto local_result = a9n::hal::current_local_variable();
            if (!local_result)
            {
                return nullptr;
            }
            auto *context = local_result.unwrap()->current_context;
            if (!context)
            {
                return nullptr;
            }
            for (a9n::word i = 0; i < a9n::hal::HARDWARE_CONTEXT_SIZE; ++i)
            {
                (*context)[i] = frame.registers[i];
            }
            return context;
        }

        a9n::kernel::hardware_context *current_context()
        {
            auto local_result = a9n::hal::current_local_variable();
            if (!local_result)
            {
                return nullptr;
            }
            return local_result.unwrap()->current_context;
        }

        void dispatch_irq(a9n::word irq)
        {
            if (irq == platform::GENERIC_TIMER_IRQ)
            {
                platform::rearm_system_timer();
                if (timer_handler)
                {
                    timer_handler();
                }
                else
                {
                    platform::end_of_interrupt(irq);
                    active_irq = 1023;
                }
                return;
            }

            if (irq == IPI_RESCHEDULE_ID)
            {
                if (ipi_reschedule_handler)
                {
                    ipi_reschedule_handler();
                }
                platform::end_of_interrupt(irq);
                active_irq = 1023;
                return;
            }

            if (irq == IPI_INVALIDATE_TLB_ID)
            {
                invalidate_tlb_all();
                platform::end_of_interrupt(irq);
                active_irq = 1023;
                return;
            }

            if (interrupt_dispatcher)
            {
                interrupt_dispatcher(irq);
            }
            else
            {
                platform::end_of_interrupt(irq);
                active_irq = 1023;
            }
        }

        void dispatch_user_sync(const exception_frame &frame)
        {
            const auto exception_class = (frame.esr_el1 >> 26) & 0x3f;
            if (exception_class == 0x15) // SVC from AArch64.
            {
                if (!kernel_call_handler)
                {
                    fatal_exception(frame);
                }
                kernel_call_handler(
                    static_cast<a9n::kernel::kernel_call_type>(
                        static_cast<a9n::sword>(frame.registers[register_index::X8])
                    )
                );
                return;
            }

            if (!fault_dispatcher)
            {
                fatal_exception(frame);
            }

            auto fault = a9n::kernel::fault_type::ARCHITECTURE;
            switch (exception_class)
            {
                case 0x20 : // Instruction abort from lower EL.
                case 0x21 : // Instruction abort from the same EL.
                    fault = a9n::kernel::fault_type::MEMORY_INSTRUCTION_FETCH;
                    break;
                case 0x24 : // Data abort from lower EL.
                case 0x25 : // Data abort from the same EL.
                    fault = a9n::kernel::fault_type::MEMORY;
                    break;
                case 0x00 : // Unknown reason generally represents an undefined instruction.
                    fault = a9n::kernel::fault_type::INVALID_INSTRUCTION;
                    break;
                default :
                    break;
            }

            fault_dispatcher(fault, static_cast<a9n::sword>(exception_class), frame.esr_el1, frame.far_el1);
        }
    }

    extern "C" a9n::word *aarch64_handle_exception(exception_frame *frame, a9n::word vector_kind)
    {
        if (!frame)
        {
            for (;;)
            {
                asm volatile("wfe");
            }
        }

        save_current_context(*frame);

        switch (vector_kind)
        {
            case 2 : // Synchronous exception from EL0.
                dispatch_user_sync(*frame);
                break;
            case 1 : // IRQ while executing EL1 (normally IDLE).
            case 3 : // IRQ from EL0.
                active_irq = platform::acknowledge_irq();
                if (active_irq < 1020)
                {
                    dispatch_irq(active_irq);
                }
                break;
            default :
                fatal_exception(*frame);
        }

        auto *context = current_context();
        if (!context)
        {
            fatal_exception(*frame);
        }
        return context->data();
    }
}

namespace a9n::hal
{
    hal_result register_system_timer_handler(interrupt_handler handler)
    {
        if (!handler)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }
        aarch64::timer_handler = handler;
        return {};
    }

    hal_result register_ipi_reschedule_handler(a9n::kernel::ipi_reschedule_handler handler)
    {
        if (!handler)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }
        aarch64::ipi_reschedule_handler = handler;
        return {};
    }

    hal_result register_kernel_call_handler(kernel_call_handler handler)
    {
        if (!handler)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }
        aarch64::kernel_call_handler = handler;
        return {};
    }

    hal_result register_interrupt_dispatcher(a9n::kernel::interrupt_dispatcher dispatcher)
    {
        if (!dispatcher)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }
        aarch64::interrupt_dispatcher = dispatcher;
        return {};
    }

    hal_result register_fault_dispatcher(a9n::kernel::fault_dispatcher dispatcher)
    {
        if (!dispatcher)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }
        aarch64::fault_dispatcher = dispatcher;
        return {};
    }

    hal_result enable_interrupt(a9n::word irq_number)
    {
        return aarch64::platform::enable_irq(irq_number);
    }

    hal_result disable_interrupt(a9n::word irq_number)
    {
        return aarch64::platform::disable_irq(irq_number);
    }

    hal_result enable_interrupt_all()
    {
        asm volatile("msr daifclr, #2" ::: "memory");
        return {};
    }

    hal_result disable_interrupt_all()
    {
        asm volatile("msr daifset, #2" ::: "memory");
        return {};
    }

    hal_result ack_interrupt()
    {
        if (aarch64::active_irq >= 1020)
        {
            return hal_error::TRY_AGAIN;
        }
        aarch64::platform::end_of_interrupt(aarch64::active_irq);
        aarch64::active_irq = 1023;
        return {};
    }

    hal_result send_ipi(ipi_type type, a9n::word core_number)
    {
        const auto sgi
            = type == ipi_type::RESCHEDULE ? aarch64::IPI_RESCHEDULE_ID : aarch64::IPI_INVALIDATE_TLB_ID;
        return aarch64::platform::send_sgi(sgi, core_number);
    }
}
