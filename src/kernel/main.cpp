#include <x86_64/segment_configurator.hpp>
#include <stdint.h>
#include <cpp_dependent/new.hpp>

extern "C" int kernel_main()
{
    // TODO: test gdt things
    // init gdt and set protection mode (ring 0 - ring 3, kernel/user mode).
    // first: make GDT struct and that array.
    char buf[8];
    segment_configurator *my_segment_configurator = new((void*)buf) segment_configurator;
    my_segment_configurator->init_gdt();
    __asm volatile("hlt");
    return 2038;
}
