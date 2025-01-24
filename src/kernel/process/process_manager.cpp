#include "kernel/kernel_result.hpp"
#include <kernel/process/process_manager.hpp>

#include <hal/hal_result.hpp>
#include <hal/interface/cpu.hpp>
#include <hal/interface/process_manager.hpp>

#include <kernel/process/cpu.hpp>
#include <kernel/process/process.hpp>
#include <kernel/process/scheduler.hpp>

// TODO: remove this
#include <kernel/memory/memory_manager.hpp>

#include <kernel/utility/logger.hpp>

#include <liba9n/libc/string.hpp>
#include <stdint.h>

// HACK: Test Capability
#include <kernel/capability/capability_node.hpp>

namespace a9n::kernel
{
    kernel_result process_manager::init(void)
    {
        highest_priority = 0;

        return {};
    }

    kernel_result process_manager::handle_timer(void)
    {
        auto schedule_and_switch = [&]() -> kernel_result
        {
            return try_schedule_and_switch();
        };

        auto run_idle_if_failed = [](kernel_error e) -> kernel_result
        {
            DEBUG_LOG("start IDLE ...\n");
            a9n::hal::idle();

            // unreachable
            return {};
        };

        auto log_kernel_error = [](kernel_error e) -> kernel_result
        {
            a9n::kernel::utility::logger::printk("kernel error : %llu\n", static_cast<a9n::word>(e));
            return e;
        };

        return retrieve_current_process()
            .and_then(
                [&](process *current) -> kernel_result
                {
                    current->quantum--;
                    // the timing when quantum becomes 0 is limited and `[[unlikely]]` is allowed
                    [[unlikely]] if (current->quantum <= 0)
                    {
                        current->quantum = QUANTUM_MAX;
                        return mark_scheduled(*current)
                            .and_then(schedule_and_switch)
                            .or_else(run_idle_if_failed);
                    }

                    return {};
                }
            )
            .or_else(log_kernel_error);
    }

    // called by timer handler
    // TODO: clean this
    kernel_result process_manager::switch_context(void)
    {
        auto result
            = a9n::hal::current_local_variable()
                  .and_then(
                      [=, this](cpu_local_variable *local_variable) -> a9n::hal::hal_result
                      {
                          auto current_process = local_variable->current_process;
                          current_process->quantum--;
                          if (current_process->quantum <= 0)
                          {
                              current_process->quantum = QUANTUM_MAX;
                              scheduler_core.add_process(current_process);

                              scheduler_core.schedule()
                                  .and_then(
                                      [=, this](process *target_process
                                      ) -> liba9n::result<process *, scheduler_error>
                                      {
                                          process &preview_process = *local_variable->current_process;
                                          local_variable->current_process = target_process;

                                          auto res = a9n::hal::switch_context(
                                              preview_process,
                                              *target_process
                                          );

                                          return target_process;
                                      }
                                  )
                                  .or_else(
                                      [=, this](scheduler_error e
                                      ) -> liba9n::result<process *, scheduler_error>
                                      {
                                          utility::logger::printk(
                                              "failed schedule : %llu\n",
                                              static_cast<a9n::word>(e)
                                          );

                                          a9n::kernel::utility::logger::printk("start IDLE\n");
                                          a9n::hal::idle();

                                          return e;
                                      }
                                  );
                          }
                          return {};
                      }
                  )
                  .or_else(
                      [this](a9n::hal::hal_error e) -> a9n::hal::hal_result
                      {
                          utility::logger::printk(
                              "failed fetch local variable : %s\n",
                              a9n::hal::hal_error_to_string(e)
                          );

                          a9n::kernel::utility::logger::printk("start IDLE\n");
                          a9n::hal::idle();
                          return e;
                      }
                  );

        if (!result)
        {
            a9n::kernel::utility::logger::printk(
                "HAL error : %s\n",
                hal::hal_error_to_string(result.unwrap_error())
            );
            return kernel_error::UNEXPECTED;
        }

        return {};
    }

    kernel_result process_manager::switch_to_user(void)
    {
        auto hal_res = a9n::hal::current_local_variable().and_then(
            [=, this](cpu_local_variable *local_variable) -> a9n::hal::hal_result
            {
                a9n::kernel::utility::logger::printk("start schedule ...\n");
                if (!local_variable)
                {
                    a9n::kernel::utility::logger::error("failed to get local variable");
                    a9n::hal::idle();
                }

                if (auto result = scheduler_core.schedule())
                {
                    auto next_process = result.unwrap();
                    if (!next_process)
                    {
                        a9n::hal::idle();
                    }
                    a9n::kernel::utility::logger::printk("local variable : 0x%016llx\n", local_variable);
                    a9n::kernel::utility::logger::printk("next process : 0x%016llx\n", next_process);

                    local_variable->current_process = next_process;

                    utility::logger::printk("switch to user ...\n");
                    a9n::hal::switch_context(*next_process, *next_process); // *preview_process* is
                                                                            // not used (stub)
                    a9n::hal::restore_context(a9n::hal::cpu_mode::USER);

                    return {};
                }

                utility::logger::printk("can't schedule\n");
                return hal::hal_error::TRY_AGAIN;
            }
        );

        if (!hal_res)
        {
            utility::logger::printk("no such local variable\n");
            return kernel_error::HAL_ERROR;
        }

        return {};
    }

    kernel_result process_manager::try_schedule_and_switch(void)
    {
        return a9n::hal::current_local_variable()
            .transform_error(convert_hal_to_kernel_error)
            .and_then(
                [&](cpu_local_variable *local_variable) -> kernel_result
                {
                    return scheduler_core.schedule()
                        .transform_error(
                            [&](scheduler_error e) -> kernel_error
                            {
                                a9n::kernel::utility::logger::printk(
                                    "failed schedule : %llu\n",
                                    static_cast<a9n::word>(e)
                                );

                                return kernel_error::NO_SUCH_ADDRESS;
                            }
                        )
                        .and_then(
                            [&](process *next_process) -> kernel_result
                            {
                                bool is_switch = local_variable->current_process != next_process;
                                if (!is_switch)
                                {
                                    return {};
                                }

                                process &preview_process        = *local_variable->current_process;
                                local_variable->current_process = next_process;

                                return a9n::hal::switch_context(preview_process, *next_process)
                                    .transform_error(convert_hal_to_kernel_error);
                            }
                        );
                }
            );
    }

    kernel_result process_manager::try_direct_schedule_and_switch(process &target_process)
    {
        return a9n::hal::current_local_variable()
            .transform_error(convert_hal_to_kernel_error)
            .and_then(
                [&](cpu_local_variable *local_variable) -> kernel_result
                {
                    return scheduler_core.try_direct_schedule(&target_process)
                        .transform_error(
                            [&](scheduler_error e) -> kernel_error
                            {
                                return kernel_error::NO_SUCH_ADDRESS;
                            }
                        )
                        .and_then(
                            [&](process *next_process) -> kernel_result
                            {
                                // yield quantum to next process

                                bool is_switch = local_variable->current_process != next_process;

                                next_process->quantum += local_variable->current_process->quantum;

                                process &preview_process        = *local_variable->current_process;
                                local_variable->current_process = next_process;

                                if (!is_switch)
                                {
                                    return {};
                                }

                                return a9n::hal::switch_context(preview_process, *next_process)
                                    .transform_error(convert_hal_to_kernel_error);
                            }
                        );
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
        if (!result)
        {
            utility::logger::printk(
                "failed to retrieve process : HAL error : %s\n",
                a9n::hal::hal_error_to_string(result.unwrap_error())
            );
            return kernel_error::HAL_ERROR;
        }

        auto current_process = result.unwrap()->current_process;
        if (!current_process)
        {
            utility::logger::printk(
                "failed to retrieve process : process is empty\n",
                a9n::hal::hal_error_to_string(result.unwrap_error())
            );

            return kernel_error::NO_SUCH_ADDRESS;
        }

        return current_process;
    }

    kernel_result process_manager::mark_scheduled(process &process)
    {
        process.status  = process_status::READY;
        process.quantum = QUANTUM_MAX;

        // DEBUG_LOG("mark scheduled 0x%016llx\n", &process);
        auto result = scheduler_core.add_process(&process);
        if (!result)
        {
            utility::logger::printk("failed to add process to scheduler\n");
            return kernel_error::UNEXPECTED;
        }

        return {};
    }

    // TODO: remove this
    // 1. merge to init_process
    // 2. receive const reference without pointer
    void process_manager::create_process(const char *process_name, a9n::virtual_address entry_point_address)
    {
        uint16_t process_id = determine_process_id();

        if (process_id <= 0)
        {
            return; // impl error
        }

        process *current_process = &process_list[process_id];
        init_process(current_process, process_id, process_name, entry_point_address);
        a9n::hal::init_hardware_context(current_process->registers);
        a9n::hal::configure_general_register(
            *current_process,
            a9n::hal::register_type::INSTRUCTION_POINTER,
            entry_point_address
        );

        utility::logger::printk(
            "create process (entry : 0x%016llx)\n",
            a9n::hal::get_general_register(*current_process, a9n::hal::register_type::INSTRUCTION_POINTER)
        );

        current_process->status = process_status::READY;
        scheduler_core.add_process(current_process);
    }

    void process_manager::init_process(
        process             *process,
        process_id           target_process_id,
        const char          *process_name,
        a9n::virtual_address entry_point_address
    )
    {
        utility::logger::printk("configure process id\n");
        process->id = target_process_id;
        utility::logger::printk("configure process name\n");
        liba9n::std::strcpy(process->name, process_name);

        utility::logger::printk("configure quantum\n");
        process->status = process_status::BLOCKED;
        if (liba9n::std::strcmp(process_name, "idle") == 0)
        {
            // not working
            process->priority = 10;
        }
        else
        {
            process->priority = 0;
        }
        process->quantum = QUANTUM_MAX;

        // liba9n::std::memset(process->stack, 0, STACK_SIZE_MAX);
        utility::logger::printk("init vm\n");
        memory_manager_core.init_virtual_memory(process);
    }

    a9n::sword process_manager::determine_process_id()
    {
        // 0 == init_server id (reserved).

        for (uint16_t i = 1; i < PROCESS_COUNT_MAX; i++)
        {
            if (process_list[i].status != process_status::UNUSED)
            {
                continue;
            }
            return i;
        }

        return -1;
    }

    void process_manager::delete_process(process_id target_process_id)
    {
    }

    process *process_manager::search_process_from_id(process_id target_process_id)
    {
        if (target_process_id <= 0)
        {
            return nullptr;
        }

        return &process_list[target_process_id];
    }
}
