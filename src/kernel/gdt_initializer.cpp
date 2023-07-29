#include "gdt_initializer.hpp"
#include <stdint.h>

extern "C" void _load_gdt();
extern "C" void _load_segment_register();

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
    load_segment_register();
}

void gdt_initializer :: load_gdt()
{
    uint64_t gdtr[2];
    gdtr[0] = ((uint64_t)gdt << 16) | (sizeof(gdt) - 1);
    gdtr[1] = ((uint64_t)gdt >> 48);
    __asm__ volatile("lgdt %0" :: "m" (gdtr));
}

void gdt_initializer :: load_segment_register()
{
}

