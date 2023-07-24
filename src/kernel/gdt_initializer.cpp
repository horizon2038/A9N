#include "gdt_initializer.hpp"
#include <stdint.h>

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
    __asm__ volatile("lgdt %0" :: "m" (gdt));
}

void gdt_initializer :: load_segment_register()
{
    // TODO: set segment register
}

