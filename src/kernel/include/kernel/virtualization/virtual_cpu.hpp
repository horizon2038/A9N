#ifndef A9N_KERNEL_VIRTUAL_CPU_HPP
#define A9N_KERNEL_VIRTUAL_CPU_HPP

#include <hal/arch/arch_types.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/process/process.hpp>
#include <kernel/types.hpp>
#include <liba9n/libcxx/array>

namespace a9n::kernel
{
    using virtual_cpu_context = liba9n::std::array<uint8_t, a9n::hal::VIRTUAL_CPU_CONTEXT_SIZE>;

    struct virtual_cpu
    {
        alignas(sizeof(virtual_cpu_context)) virtual_cpu_context context;
        a9n::kernel::process *owner { nullptr };
        a9n::word             physical_core_number { 0 };
        bool                  launched { false };
    } __attribute__((packed));
}

#endif
