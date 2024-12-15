#ifndef A9N_KERNEL_KERNEL_CALL_HPP
#define A9N_KERNEL_KERNEL_CALL_HPP

#include <kernel/capability/capability_result.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/process/process.hpp>
#include <kernel/types.hpp>

#include <stdint.h>

namespace a9n::kernel
{
    enum class kernel_call_type : a9n::word
    {
        // all calls to kernel object are made via capability
        CAPABILITY_CALL,
        // calls that do not require authorization
        YIELD,
        // DEPRECATED: do not use except for debugging purposes
        DEBUG,
    };

    // +--------------+-------------------+
    // |     8bit     |  remaining bits   |
    // +--------------+-------------------+
    // | target depth | target descriptor |
    // +--------------+-------------------+

    // called from hal's kernel call handler
    void handle_kernel_call(kernel_call_type type);

    kernel_result handle_capability_call(process &current_process);
    kernel_result handle_yield(process &current_process);
    kernel_result handle_debug_call(process &current_process);
}

#endif
