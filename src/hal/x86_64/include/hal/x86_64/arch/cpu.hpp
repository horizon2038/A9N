#ifndef X86_64_CPU_HPP
#define X86_64_CPU_HPP

#include <hal/haL_result.hpp>
#include <hal/x86_64/arch/segment_configurator.hpp>
#include <hal/x86_64/interrupt/interrupt_descriptor.hpp>

#include <kernel/process/process.hpp>
#include <kernel/types.hpp>
#include <liba9n/libcxx/array>
#include <stdint.h>

// HACK: fix
extern "C" void _write_gs_base(void *gs_base_address);

namespace a9n::hal::x86_64
{
    inline constexpr a9n::word CPU_COUNT_MAX         = 8;
    inline constexpr a9n::word KERNEL_STACK_SIZE_MAX = 512;

    using kernel_stack = liba9n::std::array<a9n::word, KERNEL_STACK_SIZE_MAX>;

    // 64 : cpu cache line size
    struct alignas(64) cpu_local_variable
    {
        // kernel_stack_top is the top address of the stack.
        // NOTE: that the value is pushed downwards.
        kernel_stack             *kernel_stack_pointer;
        kernel::hardware_context *current_context;
        a9n::word                 core_number;
        alignas(8) bool is_kernel_use_context;

        // GDT (Global Descriptor Table), IDT (Interrupt Descriptor Table),
        // TSS (Task State Segment) is exists per processor-core.
        global_descriptor_table gdt;
        interrupt_descriptor_64 idt;
        task_state_segment      tss;
    } __attribute__((packed));

    namespace segment_selector
    {
        inline constexpr uint16_t KERNEL_STACK_POINTER_OFFSET = 0x00;
        inline constexpr uint16_t USER_STACK_POINTER_OFFSET   = 0x08;
        inline constexpr uint16_t GDT_OFFSET                  = 0x10;
        inline constexpr uint16_t IDT_OFFSET
            = sizeof(global_descriptor_table) + GDT_OFFSET;
        inline constexpr uint16_t TSS_OFFSET
            = sizeof(interrupt_descriptor_64) + IDT_OFFSET;
    }

    // save to gs segment.
    inline cpu_local_variable cpu_local_variables[CPU_COUNT_MAX];

    // for interrupt/syscall.
    alignas(KERNEL_STACK_SIZE_MAX
    ) inline kernel_stack kernel_stacks[CPU_COUNT_MAX];

    inline hal_result init_cpu_state(a9n::word core_number)
    {
        if (core_number >= CPU_COUNT_MAX)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        // WARN: *unsafe*
        cpu_local_variables[core_number].kernel_stack_pointer
            = reinterpret_cast<kernel_stack *>(&kernel_stacks[core_number])
            + KERNEL_STACK_SIZE_MAX;
        cpu_local_variables[core_number].core_number = core_number;

        _write_gs_base(&(cpu_local_variables[0]));

        return {};
    }

    inline a9n::word current_core_number()
    {
        // stub (single-core)
        return 0;
    }

}
#endif
