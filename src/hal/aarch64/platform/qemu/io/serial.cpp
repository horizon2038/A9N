#include <hal/aarch64/platform.hpp>
#include <hal/interface/serial.hpp>
#include <kernel/memory/memory.hpp>

namespace a9n::hal
{
    namespace
    {
        inline constexpr a9n::word UART_CLOCK = 24'000'000;

        enum register_offset : a9n::word
        {
            DATA            = 0x000,
            FLAG            = 0x018,
            INTEGER_BAUD    = 0x024,
            FRACTION_BAUD   = 0x028,
            LINE_CONTROL    = 0x02c,
            CONTROL         = 0x030,
            INTERRUPT_MASK  = 0x038,
            INTERRUPT_CLEAR = 0x044,
        };

        volatile uint32_t &uart_register(a9n::word offset)
        {
            const auto address = aarch64::platform::UART_BASE + offset;
            return *a9n::kernel::physical_to_virtual_pointer<volatile uint32_t>(address);
        }
    }

    void init_serial(a9n::word baud_rate)
    {
        if (!baud_rate)
        {
            baud_rate = 115200;
        }

        uart_register(CONTROL) = 0;
        while ((uart_register(FLAG) & (1U << 3)) != 0)
        {
        }

        const auto divisor             = UART_CLOCK / (16 * baud_rate);
        const auto remainder           = UART_CLOCK % (16 * baud_rate);
        const auto fractional          = (remainder * 64 + (8 * baud_rate)) / (16 * baud_rate);

        uart_register(INTEGER_BAUD)    = static_cast<uint32_t>(divisor);
        uart_register(FRACTION_BAUD)   = static_cast<uint32_t>(fractional & 0x3f);
        uart_register(LINE_CONTROL)    = (3U << 5) | (1U << 4);
        uart_register(INTERRUPT_MASK)  = 0;
        uart_register(INTERRUPT_CLEAR) = 0x7ff;
        uart_register(CONTROL)         = (1U << 0) | (1U << 8) | (1U << 9);
    }

    uint8_t read_serial()
    {
        while ((uart_register(FLAG) & (1U << 4)) != 0)
        {
            asm volatile("yield");
        }
        return static_cast<uint8_t>(uart_register(DATA) & 0xff);
    }

    void write_serial(char data)
    {
        while ((uart_register(FLAG) & (1U << 5)) != 0)
        {
            asm volatile("yield");
        }
        uart_register(DATA) = static_cast<uint8_t>(data);
    }

    void write_string_serial(const char *out)
    {
        if (!out)
        {
            return;
        }
        while (*out)
        {
            write_serial(*out++);
        }
    }
}
