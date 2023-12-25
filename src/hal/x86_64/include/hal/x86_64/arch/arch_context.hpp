#ifndef ARCH_CONTEXT_HPP
#define ARCH_CONTEXT_HPP

#include <stdint.h>

#include <hal/x86_64/arch/cpu.hpp>
#include <kernel/process/process.hpp>

namespace hal::x86_66
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

    struct arch_context
    {
        // architecture context is saved here.
        uint64_t stack[kernel::STACK_SIZE_MAX];
        // set to cpu_local_variable.
        // when called system call, kernel stack is doesn't reserved from
        // TSS.RSP0. then we used GS segment and accesss this.
        uint64_t system_call_stack[kernel::STACK_SIZE_MAX];
        uint64_t gs_base;
        uint64_t fs_base;
    };
}

#endif
