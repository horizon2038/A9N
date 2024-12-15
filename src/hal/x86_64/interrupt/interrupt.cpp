#include "hal/interface/interrupt.hpp"
#include "hal/hal_result.hpp"
#include <hal/x86_64/interrupt/interrupt.hpp>

#include <hal/x86_64/arch/cpu.hpp>
#include <hal/x86_64/interrupt/apic.hpp>
#include <hal/x86_64/interrupt/interrupt_descriptor.hpp>
#include <hal/x86_64/systemcall/syscall.hpp>
#include <kernel/process/process.hpp>
#include <kernel/types.hpp>

#include <kernel/utility/logger.hpp>

#include <liba9n/common/enum.hpp>

namespace a9n::hal::x86_64
{
    // size of interrupt handler (cf., _x86_64_interrupt.s)
    using interrupt_handler_asm = uint8_t[16];
    extern "C" interrupt_handler_asm _interrupt_handlers[];

    a9n::virtual_address read_cr0()
    {
        a9n::virtual_address cr0;

        asm volatile("mov %%cr0, %0" : "=r"(cr0) : :);
        return cr0;
    }

    a9n::virtual_address read_cr2()
    {
        a9n::virtual_address cr2;

        asm volatile("mov %%cr2, %0" : "=r"(cr2) : :);
        return cr2;
    }

    a9n::virtual_address read_cr3()
    {
        a9n::virtual_address cr3;

        asm volatile("mov %%cr3, %0" : "=r"(cr3) : :);
        return cr3;
    }

    a9n::virtual_address read_cr4()
    {
        a9n::virtual_address cr4;

        asm volatile("mov %%cr4, %0" : "=r"(cr4) : :);
        return cr4;
    }

    uint64_t read_user_gs_base()
    {
        uint64_t user_gs_base {};
        asm volatile("swapgs; rdgsbase %0; swapgs" : "=r"(user_gs_base)::);
        return user_gs_base;
    }

    uint64_t read_kernel_gs_base()
    {
        uint64_t kernel_gs_base {};
        asm volatile("rdgsbase %0" : "=r"(kernel_gs_base)::);
        return kernel_gs_base;
    }

    a9n::kernel::hardware_context *current_context()
    {
        a9n::kernel::hardware_context *context;

        asm volatile("mov %%gs:0x08, %0" : "=r"(context) : :);

        return context;
    }

    void print_page_fault_reason(uint64_t error_code)
    {
        using a9n::kernel::utility::logger;

        bool is_present           = error_code >> 0 & 1;
        bool is_write             = error_code >> 1 & 1;
        bool is_user              = error_code >> 2 & 1;
        bool is_reserved_write    = error_code >> 3 & 1;
        bool is_instruction_fetch = error_code >> 4 & 1;
        bool is_protection_key    = error_code >> 5 & 1;
        bool is_shadow_stack      = error_code >> 6 & 1;
        bool is_sgx               = error_code >> 7 & 1;

        logger::printk("is_present \e[52G : %s\n", is_present ? "Y" : "N");
        logger::printk("is_write \e[52G : %s\n", is_write ? "Y" : "N");
        logger::printk("is_user \e[52G : %s\n", is_user ? "Y" : "N");
        logger::printk("is_reserved_write \e[52G : %s\n", is_reserved_write ? "Y" : "N");
        logger::printk("is_instruction_fetch \e[52G : %s\n", is_instruction_fetch ? "Y" : "N");
        logger::printk("is_protection_key \e[52G : %s\n", is_protection_key ? "Y" : "N");
        logger::printk("is_shadow_stack \e[52G : %s\n", is_shadow_stack ? "Y" : "N");
        logger::printk("is_sgx \e[52G : %s\n", is_sgx ? "Y" : "N");
    }

    inline constexpr const char *register_names[]
        = { "RAX", "RBX", "RCX", "RDX", "RDI", "RSI", "RBP",    "R8",  "R9", "R10",     "R11",
            "R12", "R13", "R14", "R15", "RIP", "CS",  "RFLAGS", "RSP", "SS", "GS_BASE", "FS_BASE" };

    void print_registers()
    {
        auto context = current_context();

        for (a9n::word i = 0; i < 22; i++)
        {
            a9n::word value = (*context)[i];
            a9n::kernel::utility::logger::printk("%s \e[38G : 0x%016llx\n", register_names[i], value);
        }

        a9n::kernel::utility::logger::printk("CR0 \e[38G : 0x%016llx\n", read_cr0());
        a9n::kernel::utility::logger::printk("CR2 \e[38G : 0x%016llx\n", read_cr2());
        a9n::kernel::utility::logger::printk("CR3 \e[38G : 0x%016llx\n", read_cr3());
        a9n::kernel::utility::logger::printk("CR4 \e[38G : 0x%016llx\n", read_cr4());

        a9n::kernel::utility::logger::printk("kernel_gs_base : 0x%016llx\n", read_kernel_gs_base());

        a9n::kernel::utility::logger::printk("user_gs_base : 0x%016llx\n", read_user_gs_base());
        a9n::kernel::utility::logger::split();
    }

    // called from asm
    extern "C" void do_irq_from_kernel(uint16_t irq_number, uint64_t error_code)
    {
        const char *exception_type = get_exception_type_string(irq_number);

        switch (irq_number)
        {
            // timer
            case 0x20 :
                a9n::kernel::utility::logger::printk(
                    "[kernel -> kernel] exception [%2d] : %s : %llu\n",
                    static_cast<int>(irq_number),
                    exception_type,
                    error_code
                );
                interrupt_handler_table[32]();
                break;

            default :
                print_registers();
                a9n::kernel::utility::logger::printk(
                    "[kernel -> kernel] exception [%2d] : %s : %llu\n",
                    static_cast<int>(irq_number),
                    exception_type,
                    error_code
                );
                print_registers();
                for (;;)
                    ;
        }

        _restore_kernel_context();
    }

    // called from asm
    extern "C" void do_irq_from_user(uint64_t irq_number, uint64_t error_code)
    {
        // a9n::kernel::utility::logger::printh("user irq!\n");
        switch (irq_number)
        {
            case 0 :
                a9n::kernel::utility::logger::printk("zero division fault! : 0x%llx\n", error_code);
                print_registers();

            // timer
            case 0x20 :
                // a9n::kernel::utility::logger::printh("timer\n");
                interrupt_handler_table[32]();
                break;

            case 13 :
                a9n::kernel::utility::logger::printk("gp fault!\n");
                print_registers();

            case 14 :
                a9n::kernel::utility::logger::printk("page fault! : 0x%llx\n", error_code);
                a9n::kernel::utility::logger::printk("fault address : 0x%016llx\n", read_cr2());
                print_page_fault_reason(error_code);
                print_registers();
                break;

            default :
                const char *exception_type = get_exception_type_string(irq_number);
                a9n::kernel::utility::logger::printk(
                    "[user -> kernel] exception [%04llu] : %s : %llu\n",
                    static_cast<int32_t>(irq_number),
                    exception_type,
                    error_code
                );
                // asm volatile("cli; hlt");
                break;
        }

        _restore_user_context();
    }

    // register initial IDT handlers; this contains the *common* code of ASM.
    // each IDT handler calls `do_irq_from_*` with the irq number as an argument.
    hal_result init_idt_handler(void)
    {
        hal_result result {};

        for (uint16_t i = 0; i < 256; i++)
        {
            result = register_idt_handler(
                i,
                reinterpret_cast<a9n::hal::interrupt_handler>(&_interrupt_handlers[i])
            );

            if (!result)
            {
                break;
            }
        }

        return result;
    }

    hal_result register_idt_handler(a9n::word irq_number, a9n::hal::interrupt_handler target_handler)
    {
        return current_arch_local_variable().and_then(
            [=](arch_cpu_local_variable *local_variable) -> hal_result
            {
                interrupt_descriptor_table &idt       = local_variable->idt;
                interrupt_descriptor_64    *idt_entry = &idt[irq_number];
                uint64_t interrupt_handler_address    = reinterpret_cast<uint64_t>(target_handler);

                idt_entry->offset_low                 = interrupt_handler_address & 0xffff;
                idt_entry->kernel_cs                  = 0x08; // kernel code segment
                idt_entry->ist                        = 1;    // kernel stack (ist_1)
                idt_entry->type                       = INTERRUPT_GATE | PRESENT | DPL_3;
                idt_entry->offset_mid                 = (interrupt_handler_address >> 16) & 0xffff;
                idt_entry->offset_high = (interrupt_handler_address >> 32) & 0xffffffff;
                idt_entry->reserved    = 0;

                return {};
            }
        );
    }

    hal_result
        ipi(uint8_t vector, ipi_delivery_mode mode, ipi_destination_shorthand shorthand, uint8_t receiver_cpu
        )
    {
        // cf., Intel SDM vol. 3A 11-19, figure 11-12, "Interrupt Command Register (ICR)"
        //      (combined : page 3403)

        // ICR high (32bit)
        // +-------------------+----------+
        // |       63-56       |  55-32   |
        // +-------------------+----------+
        // | destination field |        - |
        // +-------------------+----------+

        // ICR low (32bit)
        // clang-format off
        // +-------+-----------------------+-------+---------+-------+----+-----------------+------------------+---------------+--------+
        // | 31-20 |         19-18         | 17-16 |   15    |  14   | 13 |       12        |        11        |     10-8      |  7-0   |
        // +-------+-----------------------+-------+---------+-------+----+-----------------+------------------+---------------+--------+
        // |     - | destination shorthand |     - | trigger | level |  - | delivery status | destination mode | delivery mode | vector |
        // +-------+-----------------------+-------+---------+-------+----+-----------------+------------------+---------------+--------+
        // clang-format on

        uint32_t icr_high = receiver_cpu << 24;

        // enable "level" bits
        uint32_t icr_low = (liba9n::enum_cast(shorthand) & ((1 << 2) - 1)) << 18 | (1 << 14)
                         | (liba9n::enum_cast(mode) & ((1 << 3) - 1)) << 8 | vector;
        // uint64_t icr = icr_low | static_cast<uint64_t>(icr_high) << 32;

        return local_apic_core.write(local_apic_offset::ICR_HIGH, icr_high)
            .and_then(
                [=](void) -> hal_result
                {
                    return local_apic_core.write(local_apic_offset::ICR_LOW, icr_low);
                }
            );
    }

    hal_result ipi_init(uint8_t receiver_cpu)
    {
        return ipi(0, ipi_delivery_mode::INIT, ipi_destination_shorthand::NO_SHORTHAND, receiver_cpu);
    }

    hal_result ipi_startup(a9n::physical_address trampoline_address, uint8_t receiver_cpu)
    {
        // in ipi starup, the first address of the target trampoline code is specified in vector
        // by *physical page frame number*.
        // therefore, a 12bit shift is performed; because base is aligned to 4KiB : 2^12.

        if (trampoline_address > (4096 * UINT8_MAX))
        {
            // out of range
            return hal_error::ILLEGAL_ARGUMENT;
        }

        uint8_t page_number = trampoline_address >> 12;

        return ipi(
            page_number,
            ipi_delivery_mode::STARTUP,
            ipi_destination_shorthand::NO_SHORTHAND,
            receiver_cpu
        );
    }
}

// interface implementation
namespace a9n::hal
{
    hal_result register_interrupt_handler(a9n::word irq_number, interrupt_handler handler)
    {
        x86_64::interrupt_handler_table[irq_number] = handler;

        if (!handler)
        {
            return hal_error::NO_SUCH_ADDRESS;
        };

        a9n::kernel::utility::logger::printh(
            "register interrupt handler : %lu : 0x%016llx\n",
            irq_number,
            reinterpret_cast<uint64_t>(handler)
        );

        return {};
    }

    hal_result register_system_timer_handler(interrupt_handler handler)
    {
        if (!handler)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        x86_64::interrupt_handler_table[0x20] = handler;

        return {};
    }

    hal_result register_kernel_call_handler(kernel_call_handler handler)
    {
        if (!handler)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        x86_64::kernel_call_handler = handler;

        return {};
    }

    // enable/disable irq
    hal_result enable_interrupt(a9n::word irq_number)
    {
        /*
        return x86_64::current_arch_local_variable().and_then(
            [=](x86_64::arch_cpu_local_variable *local_variable) -> hal_result
            {
                x86_64::interrupt_descriptor_table &idt       = local_variable->idt;
                x86_64::interrupt_descriptor_64    &idt_entry = idt[irq_number];
                idt_entry.type                                = x86_64::INTERRUPT_GATE;

                return {};
            }
        );
        */

        if (irq_number >= 256)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        return x86_64::io_apic_core.enable_interrupt(irq_number);
    }

    hal_result disable_interrupt(a9n::word irq_number)
    {
        /*
        return x86_64::current_arch_local_variable().and_then(
            [=](x86_64::arch_cpu_local_variable *local_variable) -> hal_result
            {
                x86_64::interrupt_descriptor_table &idt       = local_variable->idt;
                x86_64::interrupt_descriptor_64    &idt_entry = idt[irq_number];
                idt_entry.type = x86_64::INTERRUPT_GATE | x86_64::PRESENT;

                return {};
            }
        );
        */

        if (irq_number >= 256)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        return x86_64::io_apic_core.disable_interrupt(irq_number);
    }

    hal_result enable_interrupt_all(void)
    {
        return x86_64::io_apic_core.enable_interrupt_all();
    }

    hal_result disable_interrupt_all(void)
    {
        return x86_64::io_apic_core.disable_interrupt_all();
    }

    // notify
    hal_result ack_interrupt(void)
    {
        return x86_64::local_apic_core.end_of_interrupt();
    }
}
