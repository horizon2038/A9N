#include <x86_64/segment_configurator.hpp>
#include <stdint.h>
#include <cpp_dependent/new.hpp>

#include <include/interrupt.hpp>
#include <x86_64/x86_64_interrupt.hpp>

extern "C" int kernel_main()
{
    /*
    char segment_buffer[256] = {};
    hal::x86_64::segment_configurator *my_segment_configurator = new((void*)segment_buffer) hal::x86_64::segment_configurator();
    my_segment_configurator->init_gdt();
    */

    constexpr uint16_t segment_configurator_size = sizeof(hal::x86_64::segment_configurator);
    alignas(hal::x86_64::segment_configurator) char buf[segment_configurator_size];
    hal::x86_64::segment_configurator *my_segment_configurator = new((void*)buf) hal::x86_64::segment_configurator;
    new (my_segment_configurator) hal::x86_64::segment_configurator();
    my_segment_configurator->init_gdt();

    /*
    char segment_configurator_buffer[sizeof(hal::x86_64::segment_configurator)];
    hal::x86_64::segment_configurator *my_segment_configurator = reinterpret_cast<hal::x86_64::segment_configurator*>(segment_configurator_buffer);
    new (my_segment_configurator) hal::x86_64::segment_configurator();
    my_segment_configurator->init_gdt();
    */

    asm volatile("hlt");
    return 2038;
}
