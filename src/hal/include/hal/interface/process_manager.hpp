#ifndef HAL_CONTEXT_SWITCH_HPP
#define HAL_CONTEXT_SWITCH_HPP

#include <kernel/process/process.hpp>

#include <kernel/types.hpp>

#include <hal/hal_result.hpp>

namespace a9n::hal
{
    enum class register_type
    {
        INSTRUCTION_POINTER,
        STACK_POINTER,
        THREAD_LOCAL_BASE, // TODO: implement this
    };

    enum cpu_mode
    {
        KERNEL,
        USER,
        VM
    };

    hal_result switch_context(a9n::kernel::process &next_process);
    hal_result restore_context(cpu_mode current_mode);
    void       idle(void);
    hal_result init_hardware_context(a9n::kernel::hardware_context &context);

    liba9n::result<a9n::word, hal_error>
        get_message_register(const a9n::kernel::process &target_process, a9n::word index);

    liba9n::result<a9n::word, hal_error>
        get_general_register(const a9n::kernel::process &target_process, register_type type);

    hal_result
        configure_message_register(a9n::kernel::process &target_process, a9n::word index, a9n::word value);

    hal_result configure_general_register(
        a9n::kernel::process &target_process,
        register_type         type,
        a9n::word             value
    );
}

#endif
