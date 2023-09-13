#include "interrupt.hpp"

#include "interrupt_descriptor.hpp"

namespace hal::x86_64
{
    extern "C" void _load_idt(uint16_t size, uint64_t *offset);
    extern "C" void _enable_interrupt_all();
    extern "C" void _disable_interrupt_all();

    interrupt::interrupt()
    {
    }

    interrupt::~interrupt()
    {
    }

    void interrupt::init_interrupt()
    {
        // call asm (initialize IDT)
        load_idt();
    };

    void interrupt::load_idt()
    {
        constexpr uint16_t idt_size = sizeof(idt) - 1;
        uint64_t *idt_address = (uint64_t*)idt;
        _load_idt(idt_size, idt_address);
    }

    void interrupt::register_interrupt(uint32_t irq_number, hal::interface::interrupt_handler target_interrupt_handler)
    {
        interrupt_descriptor_64 *idt_entry = &idt[irq_number];
        uint64_t interrupt_handler_address = reinterpret_cast<uint64_t>(target_interrupt_handler);

        idt_entry->offset_low = interrupt_handler_address & 0xffff;
        idt_entry->kernel_cs = 0x08; // kernel code segment
        idt_entry->ist = 0;
        idt_entry->type = INTERRUPT_GATE | PRESENT;
        idt_entry->offset_mid = (interrupt_handler_address >> 16) & 0xffff;
        idt_entry->offset_high = (interrupt_handler_address >> 32) & 0xffffffff;
        idt_entry->reserved = 0;
    };

    void interrupt::enable_interrupt(uint32_t irq_number)
    {
        interrupt_descriptor_64 *idt_entry = &idt[irq_number];

        idt_entry->type = INTERRUPT_GATE;
    };

    void interrupt::disable_interrupt(uint32_t irq_number)
    {
        interrupt_descriptor_64 *idt_entry = &idt[irq_number];

        idt_entry->type = INTERRUPT_GATE | PRESENT;
    };

    void interrupt::enable_interrupt_all()
    {
        _enable_interrupt_all();
    };

    void interrupt::disable_interrupt_all()
    {
        _disable_interrupt_all();
    };

    void interrupt::mask_interrupt(hal::interface::interrupt_mask mask) // TODO: FIX THIS
    {
        for(int i = 0; i < 256; i++)
        {
            idt[i].type = INTERRUPT_GATE | (mask.mask_bool[i] << 7);
        }
    };
}
