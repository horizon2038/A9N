#include "hal/x86_64/interrupt/interrupt_descriptor.hpp"
#include <hal/x86_64/arch/segment_configurator.hpp>
#include <stdint.h>

#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>
#include <liba9n/libc/string.hpp>

#include <hal/x86_64/arch/cpu.hpp>

namespace a9n::hal::x86_64
{
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

        load_gdt(gdt);
    }

    void segment::configure_idt(interrupt_descriptor_table &idt)
    {
        load_idt(idt);
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
        uint8_t        gdtr[10];
        const uint16_t size    = sizeof(global_descriptor_table) - 1;
        const uint64_t address = reinterpret_cast<uint64_t>(&gdt);

        liba9n::std::memcpy(&gdtr[0], &size, sizeof(size));
        liba9n::std::memcpy(&gdtr[sizeof(size)], &address, sizeof(address));

        asm volatile("lgdt %0" : : "m"(gdtr) : "memory");
    }

    void segment::load_segment_register(uint16_t code_segment_register)
    {
        const uint64_t code_selector = code_segment_register;
        const uint16_t data_selector = segment_selector::KERNEL_DS;

        asm volatile(
            "pushq %q0\n\t"
            "leaq 1f(%%rip), %%rax\n\t"
            "pushq %%rax\n\t"
            "lretq\n\t"
            "1:\n\t"
            "movw %w1, %%ax\n\t"
            "movw %%ax, %%ds\n\t"
            "movw %%ax, %%es\n\t"
            "movw %%ax, %%ss"
            :
            : "r"(code_selector), "r"(data_selector)
            : "rax", "memory"
        );
    }

    void segment::load_task_register(uint16_t segment_register)
    {
        asm volatile("ltr %w0" : : "r"(segment_register) : "memory");
    }

    void segment::load_idt(interrupt_descriptor_table &idt)
    {
        uint8_t        idtr[10];
        const uint16_t size    = sizeof(interrupt_descriptor_table) - 1;
        const uint64_t address = reinterpret_cast<uint64_t>(&idt);

        liba9n::std::memcpy(&idtr[0], &size, sizeof(size));
        liba9n::std::memcpy(&idtr[sizeof(size)], &address, sizeof(address));

        asm volatile("lidt %0" : : "m"(idtr) : "memory");
    }
}
