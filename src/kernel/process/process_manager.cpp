#include <kernel/process/process_manager.hpp>

#include <kernel/capability/address_space.hpp>
#include <kernel/config.hpp>
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
        process idle_context {};
        alignas(a9n::PAGE_SIZE) liba9n::std::array<uint8_t, a9n::PAGE_SIZE> idle_address_space_storage;

        inline kernel_result configure_idle(process &target, a9n::word core_number)
        {
            target.registers          = idle_context.registers;
            target.floating_registers = idle_context.floating_registers;
            target.quantum            = QUANTUM_MAX;
            target.priority           = 0;
            target.core_affinity      = core_number;
            target.status             = process_status::IDLE;

            return try_configure_address_space_slot(
                target.root_address_space,
                convert_slot_data_to_page_table(idle_context.root_address_space.data)
            );
        }

    }

    kernel_result init_idle_context(void)
    {
        return a9n::hal::init_hardware_context(hal::cpu_mode::KERNEL, idle_context.registers)
            .and_then(
                [](void) -> hal::hal_result
                {
                    return a9n::hal::init_floating_context(idle_context.floating_registers);
                }
            )
            .transform_error(convert_hal_to_kernel_error)
            .and_then(
                [](void) -> kernel_result
                {
                    auto physical_address = virtual_to_physical_address(
                        reinterpret_cast<a9n::virtual_address>(&idle_address_space_storage)
                    );
                    return a9n::hal::make_address_space(physical_address)
                        .transform_error(convert_hal_to_kernel_error)
                        .and_then(
                            [](page_table table) -> kernel_result
                            {
                                return try_configure_address_space_slot(
                                    idle_context.root_address_space,
                                    table
                                );
                            }
                        );
                }
            )
            .and_then(
                [](void) -> kernel_result
                {
                    return hal::configure_general_register(
                               idle_context,
                               hal::register_type::INSTRUCTION_POINTER,
                               reinterpret_cast<a9n::word>(a9n::hal::idle)
                    )
                        .transform_error(convert_hal_to_kernel_error);
                }
            );
    }

    kernel_result process_manager::init(a9n::word core_number)
    {
        highest_priority = 0;
        return configure_idle(idle_process, core_number);
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
                if (current->status != process_status::READY
                    && current->status != process_status::IDLE) [[unlikely]]
                {
                    // A remote suspend can change the state before its reschedule IPI wins
                    // arbitration against this timer interrupt.  Never re-queue that process.
                    return try_schedule_and_switch();
                }

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
                    if (local_variable->is_idle)
                    {
                        // The caller is an interrupt handler and must return so that it can issue
                        // EOI before restoring the interrupted IDLE context.
                        return {};
                    }

                    // There may be no current process during early boot. In that case, use IDLE's
                    // initialized storage while installing its address space and floating state.
                    auto &previous_process
                        = local_variable->current_process ?
                            *local_variable->current_process :
                            idle_process;
                    return a9n::hal::switch_context(previous_process, idle_process)
                        .transform_error(convert_hal_to_kernel_error)
                        .and_then(
                            [&](void) -> kernel_result
                            {
                                local_variable->current_process = &idle_process;
                                local_variable->is_idle         = true;

                                // The HAL entry path enters IDLE after this returns, after the
                                // handler-entry lock has been released.
                                return {};
                            }
                        );
                }
            );
    }

    kernel_result process_manager::try_schedule_and_switch(void)
    {
        return a9n::hal::current_local_variable()
            .transform_error(convert_hal_to_kernel_error)
            .and_then(
                [&](cpu_local_variable *local_variable) -> kernel_result
                {
                    return try_schedule_and_switch(*local_variable);
                }
            );
    }

    kernel_result process_manager::try_schedule_and_switch(cpu_local_variable &local_variable)
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
                    process &preview_process       = *local_variable.current_process;
                    local_variable.current_process = next_process;
                    local_variable.is_idle         = false;

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
            )
            .transform_error(convert_hal_to_kernel_error);
    }

    kernel_result process_manager::try_remote_target_and_switch(
        process            &target_process,
        cpu_local_variable &local_variable
    )
    {
        if (target_process.status != process_status::READY
            || !target_process.is_in_ready_queue) [[unlikely]]
        {
            return kernel_error::TRY_AGAIN;
        }

        auto remove_result = scheduler_core.remove_process(&target_process);
        if (!remove_result) [[unlikely]]
        {
            return kernel_error::UNEXPECTED;
        }

        process &preview_process        = *local_variable.current_process;
        local_variable.current_process  = &target_process;
        local_variable.is_idle          = false;

        return a9n::hal::switch_context(preview_process, target_process)
            .transform_error(convert_hal_to_kernel_error);
    }

    kernel_result process_manager::try_direct_schedule_and_switch(process &target_process)
    {
        return a9n::hal::current_local_variable()
            .transform_error(convert_hal_to_kernel_error)
            .and_then(
                [&](cpu_local_variable *local_variable) -> kernel_result
                {
                    return try_direct_schedule_and_switch(target_process, *local_variable);
                }
            );
    }

    kernel_result process_manager::try_direct_schedule_and_switch(
        process            &target_process,
        cpu_local_variable &local_variable
    )
    {
        return scheduler_core.try_direct_schedule(&target_process)
            .transform_error(
                [&](scheduler_error e) -> hal::hal_error
                {
                    return hal::hal_error::TRY_AGAIN;
                }
            )
            .and_then(
                [&](process *next_process) -> hal::hal_result
                {
                    // yield quantum to next process
                    next_process->quantum          += local_variable.current_process->quantum;

                    process &preview_process        = *local_variable.current_process;
                    local_variable.current_process  = next_process;
                    local_variable.is_idle          = false;

                    DEBUG_LOG("Switching directly to process %p ...", next_process);
                    return a9n::hal::switch_context(preview_process, *next_process);
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
        return current_process_on_this_core();
    }

    kernel_result process_manager::mark_scheduled(process &process)
    {
        process.quantum = QUANTUM_MAX;

        if (&process == &idle_process)
        {
            // IDLE is a scheduler fallback, never a member of a ready queue.
            process.status = process_status::IDLE;
            return {};
        }

        process.status = process_status::READY;

        auto result = scheduler_core.add_process(&process);
        [[unlikely]] if (!result)
        {
            if (result.unwrap_error() == scheduler_error::PROCESS_ALREADY_EXISTS_IN_QUEUE)
            {
                return {};
            }
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

    liba9n::result<process *, kernel_error> current_process_on_this_core(void)
    {
        return a9n::hal::current_local_variable()
            .transform_error(convert_hal_to_kernel_error)
            .and_then(
                [](cpu_local_variable *local_variable) -> liba9n::result<process *, kernel_error>
                {
                    if (!local_variable->current_process) [[unlikely]]
                    {
                        return kernel_error::NO_SUCH_ADDRESS;
                    }

                    return local_variable->current_process;
                }
            );
    }

    liba9n::result<process_manager *, kernel_error> current_process_manager(void)
    {
        if constexpr (!SMP_ENABLED)
        {
            return &cpu_local_variables[BSP_ID].process_manager_core;
        }

        return a9n::hal::current_local_variable()
            .transform_error(convert_hal_to_kernel_error)
            .and_then(
                [](cpu_local_variable *local_variable) -> liba9n::result<process_manager *, kernel_error>
                {
                    return &local_variable->process_manager_core;
                }
            );
    }

    kernel_result reschedule_core(a9n::word core_number)
    {
        if constexpr (SMP_ENABLED)
        {
            return a9n::hal::send_ipi(a9n::hal::ipi_type::RESCHEDULE, core_number)
                .transform_error(convert_hal_to_kernel_error);
        }

        return kernel_error::UNEXPECTED;
    }

    namespace
    {
        template<bool PREEMPT_CURRENT>
        kernel_result route_mark_scheduled(process &current, process &target)
        {
            if constexpr (!SMP_ENABLED && !PREEMPT_CURRENT)
            {
                return cpu_local_variables[BSP_ID].process_manager_core.mark_scheduled(target);
            }

            auto &manager = [&]() -> process_manager &
            {
                if constexpr (SMP_ENABLED)
                {
                    return cpu_local_variables[target.core_affinity].process_manager_core;
                }

                return cpu_local_variables[BSP_ID].process_manager_core;
            }();

            return manager.mark_scheduled(target).and_then(
                [&](void) -> kernel_result
                {
                    if constexpr (SMP_ENABLED)
                    {
                        if (target.core_affinity != current.core_affinity) [[unlikely]]
                        {
                            auto &target_local = cpu_local_variables[target.core_affinity];
                            if (!target_local.pending_reschedule_target)
                            {
                                target_local.pending_reschedule_target = &target;
                            }
                            return reschedule_core(target.core_affinity);
                        }
                    }

                    if constexpr (PREEMPT_CURRENT)
                    {
                        return manager.schedule_if_preempted_by(target);
                    }

                    return {};
                }
            );
        }
    }

    kernel_result try_schedule_and_switch(process &current)
    {
        if constexpr (!SMP_ENABLED)
        {
            return cpu_local_variables[BSP_ID].process_manager_core.try_schedule_and_switch();
        }

        auto &local_variable = cpu_local_variables[current.core_affinity];
        return local_variable.process_manager_core.try_schedule_and_switch(local_variable);
    }

    kernel_result mark_scheduled(process &current, process &target)
    {
        return route_mark_scheduled<false>(current, target);
    }

    kernel_result mark_scheduled_with_preemption(process &current, process &target)
    {
        return route_mark_scheduled<true>(current, target);
    }

    kernel_result try_direct_schedule_and_switch(process &current, process &target)
    {
        if constexpr (!SMP_ENABLED)
        {
            return cpu_local_variables[BSP_ID].process_manager_core.try_direct_schedule_and_switch(
                target
            );
        }

        auto &local_variable  = cpu_local_variables[current.core_affinity];
        auto &current_manager = local_variable.process_manager_core;
        if (target.core_affinity == current.core_affinity) [[likely]]
        {
            return current_manager.try_direct_schedule_and_switch(target, local_variable);
        }

        return cpu_local_variables[target.core_affinity]
            .process_manager_core.mark_scheduled(target)
            .and_then(
                [&](void) -> kernel_result
                {
                    auto &target_local = cpu_local_variables[target.core_affinity];
                    if (!target_local.pending_reschedule_target)
                    {
                        target_local.pending_reschedule_target = &target;
                    }
                    return reschedule_core(target.core_affinity);
                }
            )
            .and_then(
                [&](void) -> kernel_result
                {
                    return current_manager.try_schedule_and_switch(local_variable);
                }
            );
    }
}
