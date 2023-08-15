#include "x86_64_interrupt.hpp"

#include "../include/interrupt_handler.hpp"
#include "x86_64/interrupt_descriptor.hpp"

extern "C" void _load_idt(uint16_t size, uint64_t *offset);
extern "C" void _enable_interrupt_all();
extern "C" void _disable_interrupt_all();

void x86_64_interrupt :: init_interrupt()
{
    // call asm (initialize IDT)
};


void x86_64_interrupt :: load_idt()
{
    uint16_t idt_size = sizeof(x86_64_interrupt::idt) - 1;
    uint64_t *idt_address = (uint64_t*)idt;
    _load_idt(idt_size, idt_address);
}

void x86_64_interrupt :: register_interrupt(uint32_t irq_number, interrupt_handler &target_interrupt_handler)
{
    interrupt_descriptor_64 *idt_entry = &idt[irq_number];
    uint64_t interrupt_handler_address = reinterpret_cast<uint64_t>(&target_interrupt_handler);
    
    idt_entry->offset_low = interrupt_handler_address & 0xffff;
    idt_entry->kernel_cs = 0x08; // kernel code segment
    idt_entry->ist = 0;
    idt_entry->type = INTERRUPT_GATE | PRESENT;
    idt_entry->offset_mid = (interrupt_handler_address >> 16) & 0xffff;
    idt_entry->offset_high = interrupt_handler_address >> 32;
    idt_entry->reserved = 0;
};

void x86_64_interrupt :: enable_interrupt(uint32_t irq_number)
{
    interrupt_descriptor_64 *idt_entry = &idt[irq_number];
    
    idt_entry->type = INTERRUPT_GATE;

    load_idt();
};

void x86_64_interrupt :: disable_interrupt(uint32_t irq_number)
{
    interrupt_descriptor_64 *idt_entry = &idt[irq_number];

    idt_entry->type = INTERRUPT_GATE | PRESENT;

    load_idt();
};

void x86_64_interrupt :: enable_interrupt_all()
{
    _enable_interrupt_all();
};

void x86_64_interrupt :: disable_interrupt_all()
{
    _disable_interrupt_all();
};

void x86_64_interrupt :: mask_interrupt(uint64_t mask)
{
    for (int i = 0; i < 256; i++)
    {
        if ((mask & (1ull << i)) != 0)
        {
            idt[i].type &= PRESENT;
        }
    }
};
