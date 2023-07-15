#ifndef GDT_CONFIGURER_H
#define GDT_CONFIGURER_H

class gdt_configurer
{
    public:
        gdt_configurer();
        void init_gdt();
        void configure_gdt();

};

#endif
