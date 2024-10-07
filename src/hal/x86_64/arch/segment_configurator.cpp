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

    segment_configurator::segment_configurator()
    {
        // none
    }

    segment_configurator::~segment_configurator()
    {
        // none
    }

    void segment_configurator::init_gdt()
    {
        configure_gdt();
        configure_tss();
        load_gdt();
        load_segment_register(segment_selector::KERNEL_CS);
        load_task_register(segment_selector::KERNEL_TSS);

        // TODO: configure per cpu
        uint8_t *kernel_stack_top = reinterpret_cast<uint8_t *>(
            reinterpret_cast<uint8_t *>(kernel_stacks[0].data()) + 4096
        );
        configure_rsp0(reinterpret_cast<a9n::virtual_address>(kernel_stack_top));

        a9n::kernel::utility::logger::printk(
            "kernel_stack address : 0x%016llx\n",
            reinterpret_cast<a9n::virtual_address>(kernel_stack_top)
        );
    }

    void segment_configurator::configure_gdt()
    {
        gdt.kernel_code_segment  = segment_descriptor::KERNEL_CODE;
        gdt.kernel_data_segment  = segment_descriptor::KERNEL_DATA;
        gdt.user_code_segment_32 = 0;
        gdt.user_code_segment    = segment_descriptor::USER_CODE;
        gdt.user_data_segment    = segment_descriptor::USER_DATA;
    }

    void segment_configurator::configure_tss()
    {
        // +-------+-----------------+
        // | base  | tss address     |
        // +-------+-----------------+
        // | limit | sizeof(tss)     |
        // +-------+-----------------+
        // | flag  | 0x4 (size bits) |
        // +-------+-----------------+

        uint64_t tss_address = reinterpret_cast<uint64_t>(&tss);

        gdt.tss_low          = create_segment_descriptor(
            (tss_address & 0xFFFFFFFF),
            sizeof(tss),
            create_system_segment_access_field(
                true,
                false,
                system_segment_type::TSS_AVAILABLE_64
            ),
            0x4
        );
        gdt.tss_high = tss_address >> 32;
    }

    void segment_configurator::load_gdt()
    {
        uint16_t gdt_size    = sizeof(global_descriptor_table) - 1;
        uint64_t gdt_address = reinterpret_cast<uint64_t>(&gdt);
        _load_gdt(gdt_size, gdt_address);
    }

    void segment_configurator::load_segment_register(uint16_t code_segment_register)
    {
        _load_segment_register(code_segment_register);
    }

    void segment_configurator::load_task_register(uint16_t segment_register)
    {
        _load_task_register(segment_register);
    }

    void segment_configurator::configure_rsp0(a9n::virtual_address kernel_stack_pointer
    )
    {
        tss.rsp_0 = kernel_stack_pointer;
        tss.ist_1 = kernel_stack_pointer;
    }
}
