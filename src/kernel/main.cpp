#include "gdt_initializer.hpp"
#include <stdint.h>
#include <cpp_dependent/new.hpp>

extern "C" int kernel_main()
{
    // TODO: test gdt things
    // init gdt and set protection mode (ring 0 - ring 3, kernel/user mode).
    // first: make GDT struct and that array.
    char buf[255];
    gdt_initializer *my_gdt_initializer = new((void*)buf) gdt_initializer;
    my_gdt_initializer->init_gdt();
    return 2038;
}
