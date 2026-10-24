#include <kernel/process/reclamation.hpp>

#include <hal/interface/cpu.hpp>
#include <hal/interface/interrupt.hpp>
#include <hal/interface/lock.hpp>
#include <hal/interface/process_manager.hpp>
#include <kernel/config.hpp>
#include <kernel/process/cpu.hpp>
#include <kernel/process/process_manager.hpp>

namespace a9n::kernel
{
    namespace
    {
        struct stop_request
        {
            process *target { nullptr };

            enum state_type : uint8_t
            {
                IDLE,
                PENDING,
                STOPPED,
                FAILED,
            };
            volatile uint8_t state { IDLE };
        };

        // Requests are serialized by the giant lock. No per-object allocation
        // or capability_slot enlargement is required.
        stop_request stop_requests[CPU_COUNT_MAX] {};

        inline uint8_t wait_for_stop_completion(const stop_request &request)
        {
            for (;;)
            {
                const auto completion_state = a9n::hal::atomic_load(&request.state);
                if (completion_state != stop_request::PENDING)
                {
                    return completion_state;
                }
                a9n::hal::spin_wait();
            }
        }
    }

    // Called only while this CPU waits for the giant lock. The requester
    // holds that lock until we publish completion of the context switch.
    void service_pending_stop_request(kernel_entry_type entry)
    {
        auto local_result = a9n::hal::current_local_variable();
        if (!local_result)
        {
            return;
        }

        auto &current_cpu = *local_result.unwrap();
        auto &request = stop_requests[current_cpu.core_number];
        if (a9n::hal::atomic_load(&request.state) != stop_request::PENDING)
        {
            return;
        }

        auto *target = request.target;
        uint8_t completion_state = stop_request::STOPPED;
        if (current_cpu.current_process == target && !current_cpu.is_idle)
        {
            // IDLE's empty Cspace rejects the abandoned capability call
            // through the existing root check. Normal syscall entry needs
            // no additional reclamation check.
            auto switch_result = current_cpu.process_manager_core.switch_to_idle();
            if (!switch_result)
            {
                completion_state = stop_request::FAILED;
            }
            else if (entry == kernel_entry_type::SYSCALL)
            {
                // No operation ran before this lock acquisition. Re-enter it
                // on resume instead of falling through with an unset result.
                a9n::hal::restart_syscall(*target);
            }
        }

        if (current_cpu.pending_reschedule_target == target)
        {
            current_cpu.pending_reschedule_target = nullptr;
        }

        // ACK only after the context/page-table switch is done.
        a9n::hal::atomic_store(&request.state, completion_state);
    }

    kernel_result stop_process_for_reclamation(process &target)
    {
        if (target.core_affinity >= CPU_COUNT_MAX)
        {
            return kernel_error::ILLEGAL_ARGUMENT;
        }
        auto local_result = a9n::hal::current_local_variable();
        if (!local_result)
        {
            return convert_hal_to_kernel_error(local_result.unwrap_error());
        }
        auto &current_cpu = *local_result.unwrap();
        auto &target_cpu = cpu_local_variables[target.core_affinity];

        // First remove the target from scheduling, before retiring its context.
        auto suspend_result = target_cpu.process_manager_core.mark_suspended(target);
        if (!suspend_result)
        {
            return suspend_result;
        }
        if (target_cpu.pending_reschedule_target == &target)
        {
            target_cpu.pending_reschedule_target = nullptr;
        }

        if (target_cpu.current_process != &target || target_cpu.is_idle)
        {
            return {};
        }
        if (target_cpu.core_number == current_cpu.core_number)
        {
            return target_cpu.process_manager_core.switch_to_idle();
        }

        if constexpr (SMP_ENABLED)
        {
            auto &request = stop_requests[target_cpu.core_number];
            // Send before publishing: a failed IPI must not leave a request
            // that the remote CPU could still consume. Its entry cannot pass
            // the giant lock while this CPU publishes the request below.
            auto send_result = a9n::hal::send_ipi(a9n::hal::ipi_type::RESCHEDULE, target_cpu.core_number);
            if (!send_result)
            {
                return convert_hal_to_kernel_error(send_result.unwrap_error());
            }
            request.target = &target;
            a9n::hal::atomic_store(&request.state, stop_request::PENDING);

            const auto completion_state = wait_for_stop_completion(request);
            a9n::hal::atomic_store(&request.state, stop_request::IDLE);
            if (completion_state != stop_request::STOPPED)
            {
                return kernel_error::UNEXPECTED;
            }
            return {};
        }
        return kernel_error::UNEXPECTED;
    }
}
