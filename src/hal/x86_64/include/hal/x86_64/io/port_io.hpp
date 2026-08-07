#ifndef X86_64_PORT_IO_HPP
#define X86_64_PORT_IO_HPP

#include <hal/interface/port_io.hpp>

namespace a9n::hal::x86_64
{
    inline uint8_t port_read_8(uint16_t address)
    {
        uint8_t data;
        asm volatile("inb %w1, %0" : "=a"(data) : "Nd"(address) : "memory");
        return data;
    }

    inline void port_write_8(uint16_t address, uint8_t data)
    {
        asm volatile("outb %0, %w1" : : "a"(data), "Nd"(address) : "memory");
    }

    inline uint16_t port_read_16(uint16_t address)
    {
        uint16_t data;
        asm volatile("inw %w1, %0" : "=a"(data) : "Nd"(address) : "memory");
        return data;
    }

    inline void port_write_16(uint16_t address, uint16_t data)
    {
        asm volatile("outw %0, %w1" : : "a"(data), "Nd"(address) : "memory");
    }

    inline uint32_t port_read_32(uint16_t address)
    {
        uint32_t data;
        asm volatile("inl %w1, %0" : "=a"(data) : "Nd"(address) : "memory");
        return data;
    }

    inline void port_write_32(uint16_t address, uint32_t data)
    {
        asm volatile("outl %0, %w1" : : "a"(data), "Nd"(address) : "memory");
    }

    inline void port_wait(void)
    {
        asm volatile("outb %%al, $0x80" : : "a"(0));
    }

    class port_io final : public a9n::hal::port_io
    {
      public:
        uint8_t read(uint16_t address) override;
        void    write(uint16_t address, uint8_t data) override;

        inline static void io_wait()
        {
            do
            {
            } while (0);
        }
    };
}

#endif
