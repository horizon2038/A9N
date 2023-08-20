#ifndef GDT_INITIALIZER_HPP
#define GDT_INITIALIZER_HPP

#include <stdint.h>

namespace hal::x86_64
{
    class segment_configurator
    {
        public:
            segment_configurator();
            ~segment_configurator();
            void init_gdt();

        private:
            void load_gdt();
            void load_segment_register(uint16_t code_segment_register);

            constexpr static uint64_t gdt[] =
            {
                // Null Descriptor
                0x0000000000000000,
                // Kernel Mode Code Segment
                0x00af9a000000ffff,
                // Kernel Mode Data Segment
                0x00cf93000000ffff,
                // User Mode Code Segment
                0x00afaa000000ffff,
                // User Mode Data Segment
                0x00cff2000000ffff 
                /*
                0x0000000000000000,
                0x00009a000000ffff,
                0x000093000000ffff,
                0x00cf9a000000ffff,
                0x00cf93000000ffff,
                0x00af9b000000ffff,
                0x00af93000000ffff,
                0x00affb000000ffff,
                0x00aff3000000ffff
                */
            };
    };

}
#endif
