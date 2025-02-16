#include <hal/interface/interrupt.hpp>

#include <hal/hal_result.hpp>
#include <hal/x86_64/interrupt/interrupt.hpp>

#include <hal/x86_64/arch/arch_context.hpp>
#include <hal/x86_64/arch/cpu.hpp>
#include <hal/x86_64/interrupt/apic.hpp>
#include <hal/x86_64/interrupt/interrupt_descriptor.hpp>
#include <hal/x86_64/systemcall/syscall.hpp>
#include <hal/x86_64/time/acpi_pm_timer.hpp>

// HACK
#include <hal/x86_64/io/port_io.hpp>
#include <hal/x86_64/io/serial.hpp>

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

        logger::printh("===== PAGE FAULT HAS OCCURED =====\n");
        logger::printh("fault address : 0x%016llx\n", read_cr2());
        logger::printh("- present           : %s\n", is_present ? "Y" : "N");
        logger::printh("- write             : %s\n", is_write ? "Y" : "N");
        logger::printh("- user              : %s\n", is_user ? "Y" : "N");
        logger::printh("- reserved_write    : %s\n", is_reserved_write ? "Y" : "N");
        logger::printh("- instruction_fetch : %s\n", is_instruction_fetch ? "Y" : "N");
        logger::printh("- protection_key    : %s\n", is_protection_key ? "Y" : "N");
        logger::printh("- shadow_stack      : %s\n", is_shadow_stack ? "Y" : "N");
        logger::printh("- sgx               : %s\n", is_sgx ? "Y" : "N");
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
            a9n::kernel::utility::logger::printh("%s : 0x%016llx\n", register_names[i], value);
        }

        a9n::kernel::utility::logger::printh("CR0 : 0x%016llx\n", read_cr0());
        a9n::kernel::utility::logger::printh("CR2 : 0x%016llx\n", read_cr2());
        a9n::kernel::utility::logger::printh("CR3 : 0x%016llx\n", read_cr3());
        a9n::kernel::utility::logger::printh("CR4 : 0x%016llx\n", read_cr4());
        a9n::kernel::utility::logger::printh("kernel_gs_base : 0x%016llx\n", read_kernel_gs_base());
        a9n::kernel::utility::logger::printh("user_gs_base : 0x%016llx\n", read_user_gs_base());
        a9n::kernel::utility::logger::split();
    }

    // called from asm
    extern "C" void do_irq_from_kernel(uint16_t irq_number, uint64_t error_code)
    {
        const char *exception_type = get_exception_type_string(irq_number);

        switch (irq_number)
        {
            default :
                a9n::kernel::utility::logger::printh(
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
    extern "C" void do_irq_from_user(uint16_t irq_number, uint64_t error_code)
    {
        // exception
        if (irq_number < liba9n::enum_cast(reserved_irq::IO_BASE))
        {
            auto                  fault         = a9n::kernel::fault_type::NONE;
            a9n::word             fault_code    = 0; // reserved (currently unused)
            a9n::physical_address fault_address = (*current_context())[x86_64::register_index::RIP];

            switch (auto type = static_cast<exception_type>(irq_number))
            {
                case exception_type::DIVISION_ERROR :
                case exception_type::OVERFLOW :
                case exception_type::X87_FLOATING_POINT_EXCEPTION :
                    fault = a9n::kernel::fault_type::INVALID_ARITHMETIC;
                    break;

                [[likely]] case exception_type::PAGE_FAULT :
                    fault         = a9n::kernel::fault_type::MEMORY;
                    fault_address = read_cr2(); // overwrite
                    print_page_fault_reason(error_code);
                    break;

                case exception_type::INVALID_OPCODE :
                    fault = a9n::kernel::fault_type::INVALID_INSTRUCTION;
                    break;

                case exception_type::NMI :
                case exception_type::BOUND_RANGE_EXCEEDED :
                case exception_type::DEVICE_NOT_AVAILABLE :
                    // TODO: implement lazy-fpu context switch
                case exception_type::DOUBLE_FAULT :
                case exception_type::COPROCESSOR_SEGMENT_OVERRUN :
                case exception_type::INVALID_TSS :
                case exception_type::SEGMENT_NOT_PRESENT :
                case exception_type::STACK_SEGMENT_FAULT :
                case exception_type::GENERAL_PROTECTION_FAULT :
                case exception_type::ALIGNMENT_CHECK :
                case exception_type::MACHINE_CHECK :
                case exception_type::SIMD_FLOATING_POINT_EXCEPTION :
                    // TODO: implement lazy-fpu context switch
                case exception_type::VIRTUALIZATION_EXCEPTION :
                case exception_type::CONTROL_PROTECTION_EXCEPTION :
                case exception_type::HYPERVISOR_INJECTION_EXCEPTION :
                case exception_type::VMM_COMMUNICATION_EXCEPTION :
                case exception_type::SECURITY_EXCEPTION :
                case exception_type::BREAKPOINT :
                case exception_type::DEBUG :
                    a9n::kernel::utility::logger::printh("===== FATAL FAULT HAS OCCURED =====\n");
                    a9n::kernel::utility::logger::printh(
                        "exception [%2d] : %s : %llu\n",
                        irq_number,
                        get_exception_type_string(irq_number),
                        error_code
                    );
                    print_registers();
                    fault = a9n::kernel::fault_type::FATAL;
                    break;

                default :
                    break;
            }

            fault_dispatcher(fault, 0, fault_address);
        }
        else
        {
            ack_interrupt();
            switch (auto type = static_cast<reserved_irq>(irq_number))
            {
                case reserved_irq::TIMER :
                    timer_handler();
                    break;

                case reserved_irq::IPI_HALT :
                    DEBUG_LOG("IPI HALT");
                    // TODO
                    break;

                case reserved_irq::IPI_RESCHEDULE :
                    DEBUG_LOG("IPI RESCHEDULE");
                    // TODO
                    break;

                case reserved_irq::CONSOLE_0 :
                    {
                        DEBUG_LOG("UART IRQ (CONSOLE_0) occurred\n");
                        DEBUG_LOG("read : 0xc\n", read_serial());
                        break;
                    }

                default :
                    a9n::kernel::utility::logger::printh("unknown irq : [ 0x%4llu ]\n", irq_number);
                    interrupt_dispatcher(irq_number);
                    break;
            }
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

    hal_result ipi(
        uint8_t                   vector,
        ipi_delivery_mode         mode,
        ipi_destination_shorthand shorthand,
        uint8_t                   receiver_cpu,
        ipi_trigger_mode          trigger_mode,
        ipi_level_state           level_state
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

        uint32_t icr_high = static_cast<uint32_t>(receiver_cpu) << 24;

        // enable "level" bits (1<<14 => 0<<11)
        uint32_t icr_low = 0;

        // Destination Shorthand
        icr_low |= (static_cast<uint32_t>(shorthand) & 0x03) << 18;

        // Delivery Mode
        icr_low |= (static_cast<uint32_t>(mode) & 0x07) << 8;

        if (mode == ipi_delivery_mode::INIT)
        {
            icr_low |= (1 << 14); // Trigger Mode = Level
        }
        else if (mode == ipi_delivery_mode::STARTUP)
        {
            // SIPI: Trigger Mode = Edge, Level State = Deassert, Vector = Page Number
            // ICR_LOW = 0x00004600 | page_number
            icr_low |= (1 << 14); // Trigger Mode = Edge
            // Level State = 0 (Deassert)
            icr_low |= (vector & 0xFF); // Vector = Page Number
        }
        else
        {
            icr_low |= (static_cast<uint32_t>(trigger_mode) & 0x01) << 14; // Trigger Mode
            icr_low |= (static_cast<uint32_t>(level_state) & 0x01) << 15;  // Level State
            icr_low |= (vector & 0xFF);                                    // Vector
        }

        return local_apic_core.write(local_apic_offset::ICR_HIGH, icr_high)
            .and_then(
                [=](void) -> hal_result
                {
                    return local_apic_core.write(local_apic_offset::ICR_LOW, icr_low);
                }
            )
            .and_then(ipi_wait);
    }

    hal_result ipi_wait(void)
    {
        constexpr a9n::word TIMEOUT_MSEC = 100;
        a9n::word           elapsed_msec = 0;

        while ((local_apic_core.read(local_apic_offset::ICR_LOW).unwrap_or(static_cast<uint32_t>(0))
                & (1 << 12))
               != 0)
        {
            auto result = acpi_pm_timer_core.wait(1000);
            if (!result)
            {
                result.unwrap_error();
            }

            elapsed_msec++;
            if (elapsed_msec >= TIMEOUT_MSEC)
            {
                a9n::kernel::utility::logger::error("IPI timeout");
                return hal_error::TIMEOUT;
            }
        }

        return {};
    }

    hal_result ipi_init(uint8_t receiver_cpu)
    {
        using kernel::utility::logger;
        logger::printh("IPI init (APIC id : 0x%02x)\n", receiver_cpu);

        return ipi(0,
                   ipi_delivery_mode::INIT,
                   ipi_destination_shorthand::NO_SHORTHAND,
                   receiver_cpu,
                   ipi_trigger_mode::LEVEL,
                   ipi_level_state::ASSERT)
            .and_then(
                [](void) -> hal_result
                {
                    return acpi_pm_timer_core.wait(100);
                }
            )
            .and_then(
                [=](void) -> hal_result
                {
                    return ipi(
                        0,
                        ipi_delivery_mode::INIT,
                        ipi_destination_shorthand::NO_SHORTHAND,
                        receiver_cpu,
                        ipi_trigger_mode::LEVEL,
                        ipi_level_state::DEASSERT
                    );
                }
            );
    }

    hal_result ipi_startup(a9n::physical_address trampoline_address, uint8_t receiver_cpu)
    {
        // in ipi starup, the first address of the target trampoline code is specified in vector
        // by *physical page frame number*.
        // therefore, a 12bit shift is performed; because base is aligned to 4KiB : 2^12.

        using kernel::utility::logger;

        if ((trampoline_address % a9n::PAGE_SIZE) != 0)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        if (trampoline_address > (4096 * UINT8_MAX))
        {
            // out of range
            return hal_error::ILLEGAL_ARGUMENT;
        }

        uint8_t page_number = trampoline_address >> 12;

        logger::printh("IPI startup : 0x%016llx -> (APIC id : 0x%02x)\n", page_number, receiver_cpu);

        return ipi(
            page_number,
            ipi_delivery_mode::STARTUP,
            ipi_destination_shorthand::NO_SHORTHAND,
            receiver_cpu,
            ipi_trigger_mode::EDGE
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

        // x86_64::interrupt_handler_table[0x20] = handler; // TODO: remove this
        x86_64::timer_handler = handler;

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

    hal_result register_interrupt_dispatcher(a9n::kernel::interrupt_dispatcher dispatcher)
    {
        if (!dispatcher)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        x86_64::interrupt_dispatcher = dispatcher;

        return {};
    }

    hal_result register_fault_dispatcher(a9n::kernel::fault_dispatcher dispatcher)
    {
        if (!dispatcher)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        x86_64::fault_dispatcher = dispatcher;

        return {};
    }

    // enable/disable irq
    hal_result enable_interrupt(a9n::word irq_number)
    {
        if (irq_number >= 256)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        return x86_64::io_apic_core.enable_interrupt(irq_number);
    }

    hal_result disable_interrupt(a9n::word irq_number)
    {
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
