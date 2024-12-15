#ifndef A9N_HAL_X86_64_ARCH_CPU_HPP
#define A9N_HAL_X86_64_ARCH_CPU_HPP

#include <hal/interface/cpu.hpp>

#include <hal/hal_result.hpp>
#include <hal/x86_64/arch/segment_configurator.hpp>
#include <hal/x86_64/interrupt/interrupt_descriptor.hpp>

#include <kernel/process/process.hpp>
#include <kernel/types.hpp>
#include <liba9n/libcxx/array>
#include <stdint.h>

extern "C" void     _write_gs_base(uint64_t gs_base_address);
extern "C" uint64_t _read_gs_base();

extern "C" void     _write_fs_base(uint64_t fs_base_address);
extern "C" uint64_t _read_fs_base();

namespace a9n::hal::x86_64
{
    struct arch_cpu_local_variable
    {
        uint64_t local_apic_id { 0 };

        // GDT (Global Descriptor Table), IDT (Interrupt Descriptor Table),
        // TSS (Task State Segment) is exists per processor-core.
        global_descriptor_table    gdt;
        interrupt_descriptor_table idt;
        task_state_segment         tss;
    } __attribute__((packed));

    namespace cpu_local_variable_offset
    {
        inline constexpr uint64_t GDT_OFFSET = 0x00;
        inline constexpr uint64_t IDT_OFFSET = sizeof(global_descriptor_table) + GDT_OFFSET;
        inline constexpr uint64_t TSS_OFFSET = sizeof(interrupt_descriptor_64) + IDT_OFFSET;
    }

    // *x86_64 specific* cpu local variable
    inline arch_cpu_local_variable arch_cpu_local_variables[a9n::kernel::CPU_COUNT_MAX];

    hal_result init_cpu_core(void);
    hal_result init_cpu_core_features(void);
    hal_result init_cpu_core_segments(void);

    liba9n::result<arch_cpu_local_variable *, hal_error> current_arch_local_variable(void);
}

#endif
