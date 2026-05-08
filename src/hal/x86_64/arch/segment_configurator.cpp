#include "hal/x86_64/interrupt/interrupt_descriptor.hpp"
#include <hal/x86_64/arch/segment_configurator.hpp>
#include <stdint.h>

#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>
#include <liba9n/libc/string.hpp>

#include <hal/x86_64/arch/cpu.hpp>

namespace a9n::hal::x86_64
{
    extern "C" void _load_gdt(uint16_t size, uint64_t offset);
    extern "C" void _load_segment_register(uint16_t code_segment_register);
    extern "C" void _load_task_register(uint16_t segment_register);
    extern "C" void _load_idt(uint16_t size, uint64_t *offset);

    hal_result segment::init()
    {
        return current_arch_local_variable().and_then(
            [](arch_cpu_local_variable *local_variable) -> hal_result
            {
                configure_gdt(local_variable->gdt);
                configure_idt(local_variable->idt);
                configure_tss(local_variable->gdt, local_variable->tss);

                load_gdt(local_variable->gdt);
                load_segment_register(segment_selector::KERNEL_CS);
                load_task_register(segment_selector::KERNEL_TSS);
                load_idt(local_variable->idt);

                return {};
            }
        );
    }

    void segment::configure_gdt(global_descriptor_table &gdt)
    {
        gdt.kernel_code_segment  = segment_descriptor::KERNEL_CODE;
        gdt.kernel_data_segment  = segment_descriptor::KERNEL_DATA;
        gdt.user_code_segment_32 = 0;
        gdt.user_data_segment    = segment_descriptor::USER_DATA;
        gdt.user_code_segment    = segment_descriptor::USER_CODE;

        uint16_t gdt_size        = sizeof(global_descriptor_table) - 1;
        uint64_t gdt_address     = reinterpret_cast<uint64_t>(&gdt);
        _load_gdt(gdt_size, gdt_address);
    }

    void segment::configure_idt(interrupt_descriptor_table &idt)
    {
        constexpr uint64_t idt_size    = sizeof(interrupt_descriptor_table) - 1;
        uint64_t          *idt_address = reinterpret_cast<uint64_t *>(&idt);
        _load_idt(idt_size, idt_address);
    }

    void segment::configure_tss(global_descriptor_table &gdt, task_state_segment &tss)
    {
        uint64_t tss_address = reinterpret_cast<uint64_t>(&tss);

        gdt.tss_low          = create_segment_descriptor(
            (tss_address & 0xFFFFFFFF),
            sizeof(tss),
            create_system_segment_access_field(true, false, system_segment_type::TSS_AVAILABLE_64),
            0x4
        );
        gdt.tss_high = tss_address >> 32;
    }

    void segment::load_gdt(global_descriptor_table &gdt)
    {
        uint16_t gdt_size    = sizeof(global_descriptor_table) - 1;
        uint64_t gdt_address = reinterpret_cast<uint64_t>(&gdt);
        _load_gdt(gdt_size, gdt_address);
    }

    void segment::load_segment_register(uint16_t code_segment_register)
    {
        _load_segment_register(code_segment_register);
    }

    void segment::load_task_register(uint16_t segment_register)
    {
        _load_task_register(segment_register);
    }

    void segment::load_idt(interrupt_descriptor_table &idt)
    {
        constexpr uint16_t idt_size    = sizeof(idt) - 1;
        uint64_t          *idt_address = (uint64_t *)&idt;
        _load_idt(idt_size, idt_address);
    }
}
