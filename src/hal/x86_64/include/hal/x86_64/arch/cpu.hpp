#ifndef X86_64_CPU_HPP
#define X86_64_CPU_HPP

#include <hal/x86_64/arch/segment_configurator.hpp>
#include <hal/x86_64/interrupt/interrupt_descriptor.hpp>

#include <library/common/types.hpp>
#include <stdint.h>

// TODO : Research GS and FS for TLS; How to activate and management MP

namespace hal::x86_64
{
    // CPU Local Variables (CLV).
    // class cpu per CPU-1core.

    constexpr static uint16_t CPU_COUNT_MAX = 32;

    class cpu_local_variable
    {
        common::virtual_address kernel_stack_pointer;
        common::virtual_address user_stack_pointer;
        global_descriptor_table gdt;
        interrupt_descriptor_64 idt;
        task_state_segment tss;
    } __attribute__((packed));

    namespace segment_selector
    {
        constexpr static uint16_t KERNEL_STACK_POINTER_OFFSET = 0x00;
        constexpr static uint16_t USER_STACK_POINTER_OFFSET = 0x08;
        constexpr static uint16_t GDT_OFFSET = 0x10;
        constexpr static uint16_t IDT_OFFSET
            = sizeof(global_descriptor_table) + GDT_OFFSET;
    }

    // save to gs segment.
    inline static cpu_local_variable clv[CPU_COUNT_MAX];
}
#endif
