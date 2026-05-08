#ifndef INTERRUPT_DESCRIPTOR_H
#define INTERRUPT_DESCRIPTOR_H

#include <hal/interface/interrupt.hpp>
#include <liba9n/libcxx/array>
#include <stdint.h>

namespace a9n::hal::x86_64
{
    typedef struct __attribute__((packed))
    {
        uint16_t offset_low;
        uint16_t kernel_cs;
        uint8_t  ist;
        uint8_t  type;
        uint16_t offset_mid;
        uint32_t offset_high;
        uint32_t reserved;
    } interrupt_descriptor_64;

    inline constexpr uint8_t INTERRUPT_GATE = 0xe;
    inline constexpr uint8_t TRAP_GATE      = 0xf;
    inline constexpr uint8_t PRESENT        = 0x80;
    inline constexpr uint8_t DPL_3          = 0x60;

    enum class interrupt_type : uint8_t
    {
        INTERRUPT = 0xe,
        TRAP      = 0xf,
        PRESENT   = 0x80
    };

    inline constexpr uint16_t INTERRUPT_DESCRIPTOR_SIZE_MAX = 256;

    using interrupt_descriptor_table
        = liba9n::std::array<interrupt_descriptor_64, INTERRUPT_DESCRIPTOR_SIZE_MAX>;
}

#endif
