#include <kernel/process/process_manager.hpp>

#include <kernel/capability/address_space.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/memory/memory.hpp>
#include <kernel/process/cpu.hpp>
#include <kernel/process/process.hpp>
#include <kernel/process/scheduler.hpp>
#include <kernel/utility/logger.hpp>

#include <hal/hal_result.hpp>
#include <hal/interface/cpu.hpp>
#include <hal/interface/memory_manager.hpp>
#include <hal/interface/process_manager.hpp>

#include <liba9n/libc/string.hpp>
#include <stdint.h>

namespace a9n::kernel
{
    namespace
    {
        process idle_process {};

        inline kernel_result configure_idle(process &target)
        {
            alignas(a9n::PAGE_SIZE) static liba9n::std::array<uint8_t, a9n::PAGE_SIZE> idle_address_space;
            alignas(a9n::PAGE_SIZE) static liba9n::std::array<uint8_t, a9n::PAGE_SIZE> idle_stack;

            return a9n::hal::init_hardware_context(hal::cpu_mode::USER, target.registers)
                .and_then(
                    [&](void) -> hal::hal_result
                    {
                        return a9n::hal::init_floating_context(target.floating_registers);
                    }
                )
                .transform_error(convert_hal_to_kernel_error)
                .and_then(
                    [&](void) -> kernel_result
                    {
                        // address space
                        auto idle_address_space_physical = virtual_to_physical_address(
                            reinterpret_cast<a9n::virtual_address>(&idle_address_space)
                        );
                        return a9n::hal::make_address_space(idle_address_space_physical)
                            .transform_error(convert_hal_to_kernel_error)
                            .and_then(
                                [&](page_table idle_page_table) -> kernel_result
                                {
                                    return try_configure_address_space_slot(
                                        target.root_address_space,
                                        idle_page_table
                                    );
                                }
                            );
                    }
                )
                .and_then(
                    [&](void) -> kernel_result
                    {
                        target.quantum  = QUANTUM_MAX;
                        target.priority = 0; // lowest priority

                        return hal::configure_general_register(
                                   target,
                                   hal::register_type::INSTRUCTION_POINTER,
                                   reinterpret_cast<a9n::word>(a9n::hal::idle)
                        )
                            .and_then(
                                [&](void) -> hal::hal_result
                                {
                                    return hal::configure_general_register(
                                        target,
                                        hal::register_type::STACK_POINTER,
                                        reinterpret_cast<a9n::word>(&idle_stack[sizeof(idle_stack)])
                                    );
                                }
                            )
                            .transform_error(convert_hal_to_kernel_error);
                    }
                );
        }
    }

    kernel_result process_manager::init(void)
    {
        highest_priority = 0;
        return configure_idle(idle_process);
    }

    kernel_result process_manager::handle_timer(void)
    {
        auto schedule_and_switch = [&]() -> kernel_result
        {
            return try_schedule_and_switch();
        };

        auto run_idle_if_failed = [&](kernel_error e) -> kernel_result
        {
            DEBUG_LOG("start IDLE ...\n");
            switch_to_idle();

            // unreachable
            return {};
        };

        auto log_kernel_error = [](kernel_error e) -> kernel_result
        {
            a9n::kernel::utility::logger::printk("Kernel error : %llu\n", static_cast<a9n::word>(e));
            return e;
        };

        return retrieve_current_process().and_then(
            [&](process *current) -> kernel_result
            {
                current->quantum--;
                // the timing when quantum becomes 0 is limited and `[[unlikely]]` is allowed
                if (current->quantum <= 0) [[unlikely]]
                {
                    current->quantum = QUANTUM_MAX;
                    return mark_scheduled(*current).and_then(schedule_and_switch).or_else(run_idle_if_failed);
                }

                return {};
            }
        );
        // .or_else(log_kernel_error);
    }

    kernel_result process_manager::switch_to_user(void)
    {
        auto hal_res = a9n::hal::current_local_variable().and_then(
            [=, this](cpu_local_variable *local_variable) -> a9n::hal::hal_result
            {
                if (!local_variable)
                {
                    a9n::kernel::utility::logger::error("Failed to retrieve CPU local variable");
                    return hal::hal_error::NO_SUCH_ADDRESS;
                }

                if (auto result = scheduler_core.schedule())
                {
                    auto next_process = result.unwrap();
                    if (!next_process)
                    {
                        return hal::hal_error::NO_SUCH_ADDRESS;
                    }
                    DEBUG_LOG("Local variable : 0x%016llx\n", local_variable);
                    DEBUG_LOG("Next process : 0x%016llx\n", next_process);

                    local_variable->current_process = next_process;

                    utility::logger::printk("Switching to user ...\n");
                    a9n::hal::switch_context(*next_process, *next_process); // *preview_process* is
                                                                            // not used (stub)
                    a9n::hal::restore_context(a9n::hal::cpu_mode::USER);

                    return {};
                }

                utility::logger::error("Scheduling failed!");
                return hal::hal_error::TRY_AGAIN;
            }
        );

        if (!hal_res) [[unlikely]]
        {
            utility::logger::error("No such local variable");
            return kernel_error::HAL_ERROR;
        }

        return {};
    }

    kernel_result process_manager::switch_to_idle(void)
    {
        return a9n::hal::current_local_variable()
            .transform_error(convert_hal_to_kernel_error)
            .and_then(
                [&](cpu_local_variable *local_variable) -> kernel_result
                {
                    DEBUG_LOG("Switching to IDLE ...\n");
                    local_variable->current_process = &idle_process;
                    local_variable->is_idle         = true;

                    // IDLE is a kernel thread, but it appears as a User thread to enable
                    // interrupts, hlt, etc.
                    /*
                    return a9n::hal::restore_context(a9n::hal::cpu_mode::USER)
                        .transform_error(convert_hal_to_kernel_error);
                    */
                    return {};
                }
            );
    }

    kernel_result process_manager::try_schedule_and_switch(void)
    {
        return a9n::hal::current_local_variable()
            .and_then(
                [&](cpu_local_variable *local_variable) -> hal::hal_result
                {
                    return scheduler_core.schedule()
                        .transform_error(
                            [&](scheduler_error e) -> hal::hal_error
                            {
                                DEBUG_LOG("scheduler error : %s", scheduler_error_to_string(e));
                                return hal::hal_error::TRY_AGAIN;
                            }
                        )
                        .and_then(
                            [&](process *next_process) -> hal::hal_result
                            {
                                process &preview_process        = *local_variable->current_process;
                                local_variable->current_process = next_process;
                                local_variable->is_idle         = false;

                                return a9n::hal::switch_context(preview_process, *next_process);
                            }
                        )
                        .or_else(
                            [&](hal::hal_error e) -> hal::hal_result
                            {
                                DEBUG_LOG("Fallback to IDLE : %s", hal::hal_error_to_string(e));
                                return switch_to_idle().transform_error(
                                    [&](kernel_error) -> hal::hal_error
                                    {
                                        DEBUG_LOG("Failed to switch to IDLE");
                                        return hal::hal_error::TRY_AGAIN;
                                    }
                                );
                            }
                        );
                }
            )
            .transform_error(convert_hal_to_kernel_error);
    }

    kernel_result process_manager::try_direct_schedule_and_switch(process &target_process)
    {
        return a9n::hal::current_local_variable()
            .and_then(
                [&](cpu_local_variable *local_variable) -> hal::hal_result
                {
                    return scheduler_core.try_direct_schedule(&target_process)
                        .transform_error(
                            [&](scheduler_error e) -> hal::hal_error
                            {
                                return hal::hal_error::TRY_AGAIN;

                                switch_to_idle();
                            }
                        )
                        .and_then(
                            [&](process *next_process) -> hal::hal_result
                            {
                                // yield quantum to next process
                                next_process->quantum += local_variable->current_process->quantum;

                                process &preview_process        = *local_variable->current_process;
                                local_variable->current_process = next_process;
                                local_variable->is_idle         = false;

                                DEBUG_LOG("Switching directly to process %p ...", next_process);
                                return a9n::hal::switch_context(preview_process, *next_process);
                            }
                        );
                }
            )
            .transform_error(convert_hal_to_kernel_error);
    }

    kernel_result process_manager::schedule_if_preempted_by(process &target)
    {
        return retrieve_current_process().and_then(
            [&](process *current) -> kernel_result
            {
                if (!current)
                {
                    return kernel_error::NO_SUCH_ADDRESS;
                }

                if (target.priority <= current->priority)
                {
                    return {};
                }

                if (current->status == process_status::READY)
                {
                    auto result = scheduler_core.add_process(current);
                    if (!result)
                    {
                        return kernel_error::UNEXPECTED;
                    }
                }

                return try_schedule_and_switch();
            }
        );
    }

    kernel_result process_manager::yield(void)
    {
        auto schedule_and_switch = [&]() -> kernel_result
        {
            return try_schedule_and_switch();
        };

        return a9n::hal::current_local_variable()
            .transform_error(convert_hal_to_kernel_error)
            .and_then(
                [&](cpu_local_variable *local_variable) -> kernel_result
                {
                    local_variable->current_process->quantum = QUANTUM_MAX;

                    return mark_scheduled(*local_variable->current_process).and_then(schedule_and_switch);
                }
            );
    }

    liba9n::result<process *, kernel_error> process_manager::retrieve_current_process()
    {
        auto result = a9n::hal::current_local_variable();
        if (!result) [[unlikely]]
        {
            utility::logger::printk(
                "HAL error : %s\n",
                a9n::hal::hal_error_to_string(result.unwrap_error())
            );
            utility::logger::error("Failed to retrieve CPU local variable");
            return kernel_error::HAL_ERROR;
        }

        auto current_process = result.unwrap()->current_process;
        if (!current_process) [[unlikely]]
        {
            utility::logger::error("Failed to retrieve process : process is empty");
            return kernel_error::NO_SUCH_ADDRESS;
        }

        return current_process;
    }

    kernel_result process_manager::mark_scheduled(process &process)
    {
        process.status  = process_status::READY;
        process.quantum = QUANTUM_MAX;

        if (&process == &idle_process)
        {
            return {};
        }

        auto result = scheduler_core.add_process(&process);
        [[unlikely]] if (!result)
        {
            utility::logger::error("Failed to add the process to the scheduler");
            return kernel_error::UNEXPECTED;
        }

        return {};
    }

    kernel_result process_manager::mark_suspended(process &process)
    {
        auto result = scheduler_core.remove_process(&process);
        if (!result && result.unwrap_error() != scheduler_error::PROCESS_NOT_IN_QUEUE)
        {
            return kernel_error::UNEXPECTED;
        }

        process.status = process_status::BLOCKED_SUSPEND;
        return {};
    }
}
