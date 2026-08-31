#include <hal/aarch64/platform.hpp>
#include <hal/interface/serial.hpp>
#include <kernel/memory/memory.hpp>

namespace a9n::hal
{
    namespace
    {
        // The Raspberry Pi firmware fixes the PL011 input clock to 48 MHz.
        inline constexpr a9n::word UART_CLOCK = 48'000'000;

        enum gpio_register_offset : a9n::word
        {
            FUNCTION_SELECT_1 = 0x004,
            PULL_UP_DOWN_0    = 0x0e4,
        };

        enum uart_register_offset : a9n::word
        {
            DATA                  = 0x00,
            FLAG                  = 0x18,
            INTEGER_BAUD_DIVISOR  = 0x24,
            FRACTION_BAUD_DIVISOR = 0x28,
            LINE_CONTROL          = 0x2c,
            CONTROL               = 0x30,
            INTERRUPT_MASK        = 0x38,
            INTERRUPT_CLEAR       = 0x44,
        };

        volatile uint32_t &gpio_register(a9n::word offset)
        {
            const auto address = aarch64::platform::GPIO_BASE + offset;
            return *a9n::kernel::physical_to_virtual_pointer<volatile uint32_t>(address);
        }

        volatile uint32_t &uart_register(a9n::word offset)
        {
            const auto address = aarch64::platform::UART_BASE + offset;
            return *a9n::kernel::physical_to_virtual_pointer<volatile uint32_t>(address);
        }

        void configure_uart_pins()
        {
            // GPIO14 and GPIO15 use ALT0 for PL011 after disable-bt is applied.
            auto function_select = gpio_register(FUNCTION_SELECT_1);
            function_select &= ~((7U << 12) | (7U << 15));
            function_select |= (4U << 12) | (4U << 15);
            gpio_register(FUNCTION_SELECT_1) = function_select;

            auto pulls = gpio_register(PULL_UP_DOWN_0);
            pulls &= ~((3U << 28) | (3U << 30));
            gpio_register(PULL_UP_DOWN_0) = pulls;
            asm volatile("dsb sy" ::: "memory");
        }
    }

    void init_serial(a9n::word baud_rate)
    {
        if (!baud_rate)
        {
            baud_rate = 115200;
        }

        configure_uart_pins();
        uart_register(CONTROL)        = 0;
        uart_register(INTERRUPT_MASK) = 0;
        uart_register(INTERRUPT_CLEAR) = 0x7ff;

        const auto baud_denominator = 16 * baud_rate;
        const auto integer_divisor  = UART_CLOCK / baud_denominator;
        const auto remainder        = UART_CLOCK % baud_denominator;
        const auto fractional_divisor =
            (remainder * 64 + baud_denominator / 2) / baud_denominator;

        uart_register(INTEGER_BAUD_DIVISOR)  = static_cast<uint32_t>(integer_divisor);
        uart_register(FRACTION_BAUD_DIVISOR) = static_cast<uint32_t>(fractional_divisor);
        uart_register(LINE_CONTROL) = (3U << 5) | (1U << 4); // 8 data bits, FIFO enabled.
        uart_register(CONTROL) = (1U << 9) | (1U << 8) | 1U; // RX, TX, UART enabled.
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
