#ifndef ARCH_CONTEXT_HPP
#define ARCH_CONTEXT_HPP

#include <stdint.h>

#include <hal/x86_64/arch/cpu.hpp>
#include <kernel/process/process.hpp>
#include <liba9n/libcxx/array>

namespace a9n::hal::x86_64
{
    // x86_64 dependent structure.
    struct interrupt_frame
    {
        uint64_t ip;
        uint64_t cs;
        uint64_t flags;
        uint64_t sp;
        uint64_t ss;
    } __attribute__((packed));

    inline constexpr a9n::word SYSCALL_STACK_SIZE_MAX   = 128;
    inline constexpr a9n::word INTERRUPT_STACK_SIZE_MAX = 2048;

    struct arch_context
    {
        liba9n::std::array<uint8_t, INTERRUPT_STACK_SIZE_MAX> interrupt_stack;
    };

    namespace register_index
    {
        // common registers
        inline constexpr a9n::word RAX = 0;
        inline constexpr a9n::word RBX = 1;
        inline constexpr a9n::word RCX = 2;
        inline constexpr a9n::word RDX = 3;
        inline constexpr a9n::word RDI = 4;
        inline constexpr a9n::word RSI = 5;
        inline constexpr a9n::word RBP = 6;
        inline constexpr a9n::word R8  = 7;
        inline constexpr a9n::word R9  = 8;
        inline constexpr a9n::word R10 = 9;
        inline constexpr a9n::word R11 = 10;
        inline constexpr a9n::word R12 = 11;
        inline constexpr a9n::word R13 = 12;
        inline constexpr a9n::word R14 = 13;
        inline constexpr a9n::word R15 = 14;

        // interrupt(iret) frame
        // it look like you should directly set hardware_context to TSS.RSP0,
        // but this is not the case.
        // this is not so, because the offset between SS and RSP will be
        // "shifted" when an interrupt occurs in the kernel
        // (which is basically impossible).
        inline constexpr a9n::word RIP    = 15;
        inline constexpr a9n::word CS     = 16;
        inline constexpr a9n::word RFLAGS = 17;
        inline constexpr a9n::word RSP    = 18;
        inline constexpr a9n::word SS     = 19;

        // for core-local/thread-local variables
        inline constexpr a9n::word GS_BASE = 20;
        inline constexpr a9n::word FS_BASE = 21;
    }

}

#endif
