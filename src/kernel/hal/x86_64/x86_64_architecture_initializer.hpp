#ifndef X86_64_ARCHITECTURE_INITIALIZER
#define X86_64_ARCHITECTURE_INITIALIZER

#include "../include/architecture_initializer.h"


class x86_64_architecture_initializer final : public architecture_initializer
{
    public:
        x86_64_architecture_initializer();
        ~x86_64_architecture_initializer();
        void init_interrupt();
        void init_memory();
        void init_timer();
        void init_serial_device();
};

#endif
