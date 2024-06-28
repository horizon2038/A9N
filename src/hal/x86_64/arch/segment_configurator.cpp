#include <hal/x86_64/arch/segment_configurator.hpp>
#include <stdint.h>

#include <kernel/utility/logger.hpp>
#include <library/libc/string.hpp>
#include <library/common/types.hpp>

namespace hal::x86_64
{
    extern "C" void _load_gdt(uint16_t size, uint64_t *offset);
    extern "C" void _load_segment_register(uint16_t code_segment_register);
    extern "C" void _load_task_register(uint16_t segment_register);

    global_descriptor_table segment_configurator::gdt = {};
    task_state_segment      segment_configurator::tss = {};

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
    }

    void segment_configurator::configure_gdt()
    {
        gdt.null                 = 0;
        gdt.kernel_code_segment  = 0x00af9a000000ffff;
        gdt.kernel_data_segment  = 0x00cf93000000ffff;
        gdt.user_code_segment_32 = 0;
        gdt.user_code_segment    = 0x00af9a000000ffff;
        gdt.user_data_sgment     = 0x00cff2000000ffff;
    }

    void segment_configurator::configure_tss()
    {
        uint64_t tss_address = reinterpret_cast<uint64_t>(&tss);

        gdt.tss_low = 0x0000890000000000 | sizeof(tss)
                    | ((tss_address & 0xffff) << 16)
                    | (((tss_address >> 16) & 0xff) << 32)
                    | (((tss_address >> 24) & 0xff) << 56);

        gdt.tss_high = tss_address >> 32;
    }

    void segment_configurator::load_gdt()
    {
        uint16_t  gdt_size    = sizeof(gdt) - 1;
        uint64_t *gdt_address = (uint64_t *)&gdt;
        _load_gdt(gdt_size, gdt_address);
    }

    void segment_configurator::load_segment_register(
        uint16_t code_segment_register
    )
    {
        _load_segment_register(code_segment_register);
    }

    void segment_configurator::load_task_register(uint16_t segment_register)
    {
        _load_task_register(segment_register);
    }

    void segment_configurator::configure_rsp0(
        common::virtual_address kernel_stack_pointer
    )
    {
        tss.rsp_0 = kernel_stack_pointer;
    }
}
