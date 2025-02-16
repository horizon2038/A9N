#ifndef X86_64_INTERRUPT_HPP
#define X86_64_INTERRUPT_HPP

#include <hal/interface/interrupt.hpp>

#include <kernel/types.hpp>

namespace a9n::hal::x86_64
{
    alignas(sizeof(a9n::hal::interrupt_handler)
    ) inline static a9n::hal::interrupt_handler interrupt_handler_table[256];

    inline static a9n::kernel::timer_handler        timer_handler;
    inline static a9n::kernel::interrupt_dispatcher interrupt_dispatcher;
    inline static a9n::kernel::fault_dispatcher     fault_dispatcher;

    extern "C" void do_irq_from_kernel(uint16_t irq_number, uint64_t error_code);
    extern "C" void do_irq_from_user(uint16_t irq_number, uint64_t error_code);

    extern "C" void _enable_interrupt_all(void);
    extern "C" void _disable_interrupt_all(void);

    extern "C" [[noreturn]] void _restore_kernel_context(void);
    extern "C" [[noreturn]] void _restore_user_context(void);

    hal_result init_idt_handler(void);
    hal_result register_idt_handler(a9n::word irq_number, a9n::hal::interrupt_handler target_handler);

    enum class ipi_destination_shorthand : uint8_t
    {
        NO_SHORTHAND       = 0x00,
        SELF               = 0x01,
        ALL_EXCLUDING_SELF = 0x02,
        ALL_INCLUDING_SELF = 0x03,
    };

    enum class ipi_delivery_mode : uint8_t
    {
        FIXED                       = 0x00,
        LOWEST_PRIORITY             = 0x01,
        SYSTEM_MANAGEMENT_INTERRUPT = 0x02,
        NON_MASKABLE_INTERRUPT      = 0x04,
        INIT                        = 0x05,
        STARTUP                     = 0x06,
        EXTERNAL_INTERRUPT          = 0x07,
    };

    enum class ipi_trigger_mode : uint8_t
    {
        EDGE  = 0,
        LEVEL = 1,
    };

    enum class ipi_level_state : uint8_t
    {
        DEASSERT = 0,
        ASSERT   = 1,
    };

    hal_result ipi(
        uint8_t                   vector,
        ipi_delivery_mode         mode,
        ipi_destination_shorthand shorthand,
        uint8_t                   receiver_cpu,
        ipi_trigger_mode          trigger_mode,
        ipi_level_state           level_state = ipi_level_state::ASSERT
    );
    hal_result ipi_wait(void);
    hal_result ipi_init(uint8_t receiver_cpu);
    hal_result ipi_startup(a9n::physical_address trampoline_address, uint8_t receiver_cpu);

    enum class exception_type : uint16_t
    {
        DIVISION_ERROR              = 0,
        DEBUG                       = 1,
        NMI                         = 2,
        BREAKPOINT                  = 3,
        OVERFLOW                    = 4,
        BOUND_RANGE_EXCEEDED        = 5,
        INVALID_OPCODE              = 6,
        DEVICE_NOT_AVAILABLE        = 7,
        DOUBLE_FAULT                = 8,
        COPROCESSOR_SEGMENT_OVERRUN = 9,
        INVALID_TSS                 = 10,
        SEGMENT_NOT_PRESENT         = 11,
        STACK_SEGMENT_FAULT         = 12,
        GENERAL_PROTECTION_FAULT    = 13,
        PAGE_FAULT                  = 14,
        // RESERVED 15
        X87_FLOATING_POINT_EXCEPTION  = 16,
        ALIGNMENT_CHECK               = 17,
        MACHINE_CHECK                 = 18,
        SIMD_FLOATING_POINT_EXCEPTION = 19,
        VIRTUALIZATION_EXCEPTION      = 20,
        CONTROL_PROTECTION_EXCEPTION  = 21,
        // RESERVED 22-27
        HYPERVISOR_INJECTION_EXCEPTION = 28,
        VMM_COMMUNICATION_EXCEPTION    = 29,
        SECURITY_EXCEPTION             = 30,
    };

    inline constexpr const char *get_exception_type_string(uint8_t raw_type)
    {
        auto type = static_cast<exception_type>(raw_type);
        switch (type)
        {
            case exception_type::DIVISION_ERROR :
                return "division_error";

            case exception_type::DEBUG :
                return "debug";

            case exception_type::NMI :
                return "non-maskable_interrupt";

            case exception_type::BREAKPOINT :
                return "breakpoint";

            case exception_type::OVERFLOW :
                return "overflow";

            case exception_type::BOUND_RANGE_EXCEEDED :
                return "bound_range_exceeded";

            case exception_type::INVALID_OPCODE :
                return "invalid opcode";

            case exception_type::DEVICE_NOT_AVAILABLE :
                return "device_not_avilable";

            case exception_type::DOUBLE_FAULT :
                return "double_fault";

            case exception_type::COPROCESSOR_SEGMENT_OVERRUN :
                return "coprocessor_segment_overrun";

            case exception_type::INVALID_TSS :
                return "invalid_tss";

            case exception_type::SEGMENT_NOT_PRESENT :
                return "segment_not_present";

            case exception_type::STACK_SEGMENT_FAULT :
                return "stack_segment_fault";

            case exception_type::GENERAL_PROTECTION_FAULT :
                return "general_protection_fault";

            case exception_type::PAGE_FAULT :
                return "page_fault";

            case exception_type::X87_FLOATING_POINT_EXCEPTION :
                return "x87_floating_point_exception";

            case exception_type::ALIGNMENT_CHECK :
                return "alignment_check";

            case exception_type::MACHINE_CHECK :
                return "machine_check";

            case exception_type::SIMD_FLOATING_POINT_EXCEPTION :
                return "simd_floating_point_exception";

            case exception_type::VIRTUALIZATION_EXCEPTION :
                return "virtualization_exception";

            case exception_type::CONTROL_PROTECTION_EXCEPTION :
                return "control_protection_exception";

            case exception_type::HYPERVISOR_INJECTION_EXCEPTION :
                return "hypervisor_injection_exception";

            case exception_type::VMM_COMMUNICATION_EXCEPTION :
                return "vmm_communication_exception";

            case exception_type::SECURITY_EXCEPTION :
                return "security_exception";

            default :
                return "reserved";
        }
    }

    enum class reserved_irq : uint16_t
    {
        IO_BASE        = 0x20,
        IPI_HALT       = 0x21,
        IPI_RESCHEDULE = 0x22,

        CONSOLE_1      = 0x23,
        CONSOLE_0      = 0x24,

        BASE           = 0x30,
        TIMER          = BASE,
    };

    // inline interrupt interrupt_core {};

}
#endif
