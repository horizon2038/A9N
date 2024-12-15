#ifndef X86_64_INTERRUPT_HPP
#define X86_64_INTERRUPT_HPP

#include <hal/interface/interrupt.hpp>

#include <kernel/types.hpp>

namespace a9n::hal::x86_64
{
    alignas(sizeof(a9n::hal::interrupt_handler)
    ) inline static a9n::hal::interrupt_handler interrupt_handler_table[256];

    extern "C" void do_irq_from_kernel(uint16_t irq_number, uint64_t error_code);
    extern "C" void do_irq_from_user(uint64_t irq_number, uint64_t error_code);

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

    enum class ipi_delivery_mode : a9n::word
    {
        FIXED                       = 0x00,
        LOWEST_PRIORITY             = 0x01,
        SYSTEM_MANAGEMENT_INTERRUPT = 0x02,
        NON_MASKABLE_INTERRUPT      = 0x04,
        INIT                        = 0x05,
        STARTUP                     = 0x06,
        EXTERNAL_INTERRUPT          = 0x07,
    };

    hal_result
        ipi(uint8_t vector, ipi_delivery_mode mode, ipi_destination_shorthand shorthand, uint8_t receiver_cpu
        );
    hal_result ipi_init(uint8_t receiver_cpu);
    hal_result ipi_startup(a9n::physical_address trampoline_address, uint8_t receiver_cpu);

    enum class EXCEPTION_TYPE : uint16_t
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

    inline constexpr const char *get_exception_type_string(uint8_t type)
    {
        auto exception_type = static_cast<EXCEPTION_TYPE>(type);
        switch (exception_type)
        {
            case EXCEPTION_TYPE::DIVISION_ERROR :
                return "division_error";

            case EXCEPTION_TYPE::DEBUG :
                return "debug";

            case EXCEPTION_TYPE::NMI :
                return "non-maskable_interrupt";

            case EXCEPTION_TYPE::BREAKPOINT :
                return "breakpoint";

            case EXCEPTION_TYPE::OVERFLOW :
                return "overflow";

            case EXCEPTION_TYPE::BOUND_RANGE_EXCEEDED :
                return "bound_range_exceeded";

            case EXCEPTION_TYPE::INVALID_OPCODE :
                return "invalid opcode";

            case EXCEPTION_TYPE::DEVICE_NOT_AVAILABLE :
                return "device_not_avilable";

            case EXCEPTION_TYPE::DOUBLE_FAULT :
                return "double_fault";

            case EXCEPTION_TYPE::COPROCESSOR_SEGMENT_OVERRUN :
                return "coprocessor_segment_overrun";

            case EXCEPTION_TYPE::INVALID_TSS :
                return "invalid_tss";

            case EXCEPTION_TYPE::SEGMENT_NOT_PRESENT :
                return "segment_not_present";

            case EXCEPTION_TYPE::STACK_SEGMENT_FAULT :
                return "stack_segment_fault";

            case EXCEPTION_TYPE::GENERAL_PROTECTION_FAULT :
                return "general_protection_fault";

            case EXCEPTION_TYPE::PAGE_FAULT :
                return "page_fault";

            case EXCEPTION_TYPE::X87_FLOATING_POINT_EXCEPTION :
                return "x87_floating_point_exception";

            case EXCEPTION_TYPE::ALIGNMENT_CHECK :
                return "alignment_check";

            case EXCEPTION_TYPE::MACHINE_CHECK :
                return "machine_check";

            case EXCEPTION_TYPE::SIMD_FLOATING_POINT_EXCEPTION :
                return "simd_floating_point_exception";

            case EXCEPTION_TYPE::VIRTUALIZATION_EXCEPTION :
                return "virtualization_exception";

            case EXCEPTION_TYPE::CONTROL_PROTECTION_EXCEPTION :
                return "control_protection_exception";

            case EXCEPTION_TYPE::HYPERVISOR_INJECTION_EXCEPTION :
                return "hypervisor_injection_exception";

            case EXCEPTION_TYPE::VMM_COMMUNICATION_EXCEPTION :
                return "vmm_communication_exception";

            case EXCEPTION_TYPE::SECURITY_EXCEPTION :
                return "security_exception";

            default :
                return "reserved";
        }
    }

    // inline interrupt interrupt_core {};

}
#endif
