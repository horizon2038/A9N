#ifndef GDT_INITIALIZER_HPP
#define GDT_INITIALIZER_HPP

#include <stdint.h>

class gdt_initializer
{
    public:
        gdt_initializer();
        ~gdt_initializer();
        void init_gdt();

    private:
        constexpr static uint64_t gdt[] =
        {
            0x0000000000000000,
            0x00009a000000ffff,
            0x000093000000ffff,
            0x00cf9a000000ffff,
            0x00cf93000000ffff,
            0x00af9b000000ffff,
            0x00af93000000ffff,
            0x00affb000000ffff,
            0x00aff3000000ffff
        };
        void load_gdt();
        void load_segment_register(uint16_t code_segment_register);
    };

#endif
