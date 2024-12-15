#ifndef HAL_x86_64_APIC_HPP
#define HAL_x86_64_APIC_HPP

#include <kernel/types.hpp>

#include <hal/hal_result.hpp>
#include <hal/x86_64/platform/acpi.hpp>

namespace a9n::hal::x86_64
{
    // Local APIC
    namespace local_apic_offset
    {
        inline constexpr uint32_t ID_REGISTER          = 0x020;
        inline constexpr uint32_t VERSION              = 0x030;
        inline constexpr uint32_t TASK_PRIORITY        = 0x080;
        inline constexpr uint32_t ARBITRATION_PRIORITY = 0x090;
        inline constexpr uint32_t PROCESSOR_PRIORITY   = 0x0A0;
        inline constexpr uint32_t END_OF_INTERRUPT     = 0x0B0;
        inline constexpr uint32_t LOGICAL_DESITINATION = 0x0D0;
        inline constexpr uint32_t DESTINATION_FORMAT   = 0x0E0;
        inline constexpr uint32_t SPURIOUS_INTERRUPT   = 0x0F0;
        inline constexpr uint32_t ICR_LOW              = 0x300;
        inline constexpr uint32_t ICR_HIGH             = 0x310;
        inline constexpr uint32_t LVT_TIMER            = 0x320;
        inline constexpr uint32_t LVT_ERROR            = 0x370;
        inline constexpr uint32_t TIMER_INIT_COUNT     = 0x380;
        inline constexpr uint32_t TIMER_CURRENT_COUNT  = 0x390;
        inline constexpr uint32_t TIMER_DIVIDE         = 0x3E0;
    }

    namespace local_apic_flag
    {
        inline constexpr uint32_t APIC_ENABLE = 1 << 11;
    }

    // IO APIC
    namespace io_apic_address
    {
        inline constexpr uint32_t IO_REGISTER_SELECT = 0xfec00000;
        inline constexpr uint32_t IO_REGISTER_WINDOW = 0xfec00010;
    }

    namespace io_apic_offset
    {
        inline constexpr uint8_t REGISTER_SELECT = 0x00;
        inline constexpr uint8_t REGISTER_WINDOW = 0x10;
    }

    namespace io_apic_register_index
    {
        inline constexpr uint8_t ID                   = 0x00;
        inline constexpr uint8_t VERSION              = 0x01;
        inline constexpr uint8_t ARBITRATION_PRIORITY = 0x02;
        inline constexpr uint8_t REDIRECTION_TABLE    = 0x10;
    }

    class io_apic
    {
      public:
        hal_result init(madt *madt_base);

        hal_result disable_interrupt(uint8_t irq_number);
        hal_result enable_interrupt(uint8_t irq_number);

        hal_result disable_interrupt_all();
        hal_result enable_interrupt_all();

        liba9n::result<uint32_t, hal_error> read(uint32_t io_apic_register);
        hal_result                          write(uint32_t io_apic_register, uint32_t value);

      private:
        uint8_t  id;
        uint32_t base_address;
        uint32_t global_interrupt_base;

        volatile uint32_t *register_select;
        volatile uint32_t *window;

        hal_result configure_from_madt(madt *madt_base);
        hal_result configure_registers();
        hal_result configure_entry(uint8_t irq_number, uint64_t entry);
    };

    class local_apic
    {
      public:
        hal_result init();

        liba9n::result<uint32_t, hal_error> read(uint32_t offset);
        hal_result                          write(uint32_t offset, uint64_t value);

        hal_result end_of_interrupt();

      private:
        volatile uint32_t *base;
    };

    // WARN: global
    inline io_apic    io_apic_core {};
    inline local_apic local_apic_core {};

}

#endif
