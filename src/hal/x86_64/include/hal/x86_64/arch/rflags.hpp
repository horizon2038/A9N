#ifndef A9N_HAL_X86_64_RFLAGS_HPP
#define A9N_HAL_X86_64_RFLAGS_HPP

#include <kernel/types.hpp>

namespace a9n::hal::x86_64
{
    namespace rflags_flag
    {
        inline constexpr uint64_t CARRY                     = 1 << 0;
        inline constexpr uint64_t PARITY                    = 1 << 2;
        inline constexpr uint64_t AUXILIARY_CARRY           = 1 << 4;
        inline constexpr uint64_t ZERO_FLAG                 = 1 << 6;
        inline constexpr uint64_t SIGN                      = 1 << 7;
        inline constexpr uint64_t TRAP                      = 1 << 8;
        inline constexpr uint64_t INTERRUPT                 = 1 << 9;
        inline constexpr uint64_t DIRECTION                 = 1 << 10;
        inline constexpr uint64_t OVERFLOW                  = 1 << 11;
        inline constexpr uint64_t IO_PRIVILEGE_LEVEL        = 1 << 13;
        inline constexpr uint64_t NESTED_TASK               = 1 << 14;
        inline constexpr uint64_t RESUME                    = 1 << 16;
        inline constexpr uint64_t VIRTUAL_8086_MODE         = 1 << 17;
        inline constexpr uint64_t ALIGNMENT_CHECK           = 1 << 18;
        inline constexpr uint64_t ACCESS_CONTROL            = 1 << 18;
        inline constexpr uint64_t VIRTUAL_INTERRUPT         = 1 << 19;
        inline constexpr uint64_t VIRTUAL_INTERRUPT_PENDING = 1 << 20;
        inline constexpr uint64_t ID                        = 1 << 21;
    }

    extern "C" uint64_t _read_rflags(void);
}

#endif
