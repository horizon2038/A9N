#ifndef INTERRUPT_HPP
#define INTERRUPT_HPP

#include <stdint.h>

class interrupt
{
    public:
        void init();
        static void do_irq(uint8_t irq_number);
};

#endif
