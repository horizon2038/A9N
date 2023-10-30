#include "segment_configurator.hpp"
#include <stdint.h>

#include <library/logger.hpp>

#include <library/string.hpp>

namespace hal::x86_64
{
    extern "C" void _load_gdt(uint16_t size, uint64_t *offset);
    extern "C" void _load_segment_register(uint16_t code_segment_register);

    global_descriptor_table segment_configurator::gdt = {};
    tast_state_segment segment_configurator::tss = {};

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
        load_gdt();
        load_segment_register(0x08);
    }

    void segment_configurator::configure_gdt()
    {
        this->gdt.null = 0;
        this->gdt.kernel_code_segment = 0x00af9a000000ffff;
        this->gdt.kernel_data_segment = 0x00cf93000000ffff;
        this->gdt.user_code_segment_32 = 0;
        this->gdt.user_code_segment = 0x00af9a000000ffff;
        this->gdt.user_data_sgment = 0x00cff2000000ffff;
    }

    void segment_configurator::load_gdt()
    {
        uint16_t gdt_size = sizeof(gdt) - 1;
        uint64_t *gdt_address = (uint64_t*)&gdt;
        _load_gdt(gdt_size, gdt_address);
    }

    void segment_configurator::load_segment_register(uint16_t code_segment_register)
    {
        _load_segment_register(code_segment_register);
    }
}

