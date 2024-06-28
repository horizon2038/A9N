#include <hal/x86_64/interrupt/interrupt.hpp>

#include <hal/x86_64/interrupt/interrupt_descriptor.hpp>

#include <kernel/utility/logger.hpp>

namespace a9n::hal::x86_64
{
    extern "C" void _load_idt(uint16_t size, uint64_t *offset);
    extern "C" void _enable_interrupt_all();
    extern "C" void _disable_interrupt_all();

    using interrupt_handler_asm = uint8_t[16];
    extern "C" interrupt_handler_asm interrupt_handlers[];

    extern "C" void do_irq(uint16_t irq_number, uint64_t error_code)
    {
        // a9n::kernel::utility::logger::printk("irq_number : %d\n", irq_number);
        bool        is_exception   = false;
        const char *exception_type = get_exception_type_string(irq_number);

        switch (irq_number)
        {
            case 0 :
                interrupt_handler_table[irq_number]();
                break;

            default :
                a9n::kernel::utility::logger::printk(
                    "exception [%2d] : %s : %llu\n",
                    irq_number,
                    exception_type,
                    error_code
                );
        }
    }

    interrupt::interrupt() : _pic()
    {
    }

    interrupt::~interrupt()
    {
    }

    void interrupt::init_interrupt()
    {
        // call asm (initialize IDT)
        init_handler();
        a9n::kernel::utility::logger::printk("handler init\n");
        load_idt();
    };

    void interrupt::init_handler()
    {
        for (uint16_t i = 0; i < 256; i++)
        {
            register_idt_handler(
                i,
                (a9n::hal::interrupt_handler)&interrupt_handlers[i]
            );
        }
    }

    void interrupt::load_idt()
    {
        constexpr uint16_t idt_size    = sizeof(idt) - 1;
        uint64_t          *idt_address = (uint64_t *)idt;
        _load_idt(idt_size, idt_address);
        disable_interrupt_all();
    }

    void interrupt::register_idt_handler(
        a9n::word                      irq_number,
        a9n::hal::interrupt_handler target_interrupt_handler
    )
    {
        interrupt_descriptor_64 *idt_entry = &idt[irq_number];
        uint64_t                 interrupt_handler_address
            = reinterpret_cast<uint64_t>(target_interrupt_handler);

        idt_entry->offset_low  = interrupt_handler_address & 0xffff;
        idt_entry->kernel_cs   = 0x08; // kernel code segment
        idt_entry->ist         = 0;
        idt_entry->type        = INTERRUPT_GATE | PRESENT;
        idt_entry->offset_mid  = (interrupt_handler_address >> 16) & 0xffff;
        idt_entry->offset_high = (interrupt_handler_address >> 32) & 0xffffffff;
        idt_entry->reserved    = 0;
    };

    void interrupt::register_handler(
        a9n::word                      irq_number,
        a9n::hal::interrupt_handler target_interrupt_handler
    )
    {
        interrupt_handler_table[irq_number] = target_interrupt_handler;
        uint64_t interrupt_handler_address
            = reinterpret_cast<uint64_t>(target_interrupt_handler);

        a9n::kernel::utility::logger::printk(
            "hal_register_interrupt : %lu : 0x%016llx\n",
            irq_number,
            interrupt_handler_address
        );
    }

    void interrupt::enable_interrupt(a9n::word irq_number)
    {
        interrupt_descriptor_64 *idt_entry = &idt[irq_number];

        idt_entry->type = INTERRUPT_GATE;
    };

    void interrupt::disable_interrupt(a9n::word irq_number)
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

    void interrupt::ack_interrupt()
    {
        _pic.end_of_interrupt_pic(0);
    }

}
