#ifndef A9N_KERNEL_PROCESS_CPU_HPP
#define A9N_KERNEL_PROCESS_CPU_HPP

#include <kernel/process/process.hpp>
#include <kernel/types.hpp>
#include <kernel/virtualization/virtual_cpu.hpp>
#include <liba9n/libcxx/array>
#include <liba9n/result/result.hpp>

namespace a9n::kernel
{
    inline constexpr a9n::word CPU_COUNT_MAX         = 64;
    inline constexpr a9n::word KERNEL_STACK_SIZE_MAX = a9n::PAGE_SIZE;

    using kernel_stack = liba9n::std::array<uint8_t, KERNEL_STACK_SIZE_MAX>;

    struct alignas(sizeof(a9n::word)) cpu_local_variable
    {
        // kernel_stack_top is the top address of the stack.
        // NOTE: that the value is pushed downwards.
        uint8_t *kernel_stack_pointer;

        union
        {
            // hardware_context is a first member of process
            process          *current_process;
            hardware_context *current_context;
        };

        a9n::kernel::virtual_cpu *current_virtual_cpu;

        a9n::word core_number;
        alignas(sizeof(a9n::word)) bool is_idle { false };
    } __attribute__((packed));

    inline cpu_local_variable cpu_local_variables[CPU_COUNT_MAX];
    inline kernel_stack       kernel_stacks[CPU_COUNT_MAX];

    // kernel-level initialization
    inline void init_cpu_local_variable()
    {
        for (auto i = 0; i < CPU_COUNT_MAX; i++)
        {
            // *unsafe* configure kernel stack top
            uint8_t *kernel_stack_pointer = kernel_stacks[i].data() + KERNEL_STACK_SIZE_MAX;
            cpu_local_variables[i].kernel_stack_pointer = kernel_stack_pointer;

            // 1:1 map
            cpu_local_variables[i].core_number = i;
        }
    }
}

#endif
