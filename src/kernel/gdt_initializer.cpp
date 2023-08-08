#include "gdt_initializer.hpp"
#include <stdint.h>

extern "C" void _load_gdt(uint16_t size, uint64_t *offset);
extern "C" void _load_segment_register(uint16_t code_segment_register);

gdt_initializer :: gdt_initializer()
{
    // none
}

gdt_initializer :: ~gdt_initializer()
{
    // none
}

void gdt_initializer :: init_gdt()
{
    load_gdt();
    load_segment_register(0x08);
}

void gdt_initializer :: load_gdt()
{
    uint16_t gdt_size = sizeof(gdt) - 1;
    uint64_t *gdt_address = gdt;
    _load_gdt(gdt_size, gdt_address);
}

void gdt_initializer :: load_segment_register(uint16_t code_segment_register)
{
    _load_segment_register(code_segment_register);
}

