#include "segment_configurer.hpp"
#include <stdint.h>

extern "C" void _load_gdt(uint16_t size, uint64_t *offset);
extern "C" void _load_segment_register(uint16_t code_segment_register);

segment_configurer :: segment_configurer()
{
    // none
}

segment_configurer :: ~segment_configurer()
{
    // none
}

void segment_configurer :: init_gdt()
{
    load_gdt();
    load_segment_register(0x08);
}

void segment_configurer :: load_gdt()
{
    uint16_t gdt_size = sizeof(gdt) - 1;
    uint64_t *gdt_address = (uint64_t*)gdt;
    _load_gdt(gdt_size, gdt_address);
}

void segment_configurer :: load_segment_register(uint16_t code_segment_register)
{
    _load_segment_register(code_segment_register);
}

