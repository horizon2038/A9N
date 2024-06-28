#include <kernel/kernel.hpp>

#include <kernel/interrupt/interrupt_manager.hpp>
#include <kernel/ipc/ipc_manager.hpp>
#include <kernel/process/process.hpp>
#include <kernel/process/process_manager.hpp>

#include <kernel/memory/memory_manager.hpp>

namespace a9n::kernel
{
    alignas(a9n::kernel::memory_manager
    ) char kernel_object::memory_manager_buffer[memory_manager_size];
    a9n::kernel::memory_manager *kernel_object::memory_manager = nullptr;

    alignas(a9n::kernel::interrupt_manager
    ) char kernel_object::interrupt_manager_buffer[interrupt_manager_size];
    a9n::kernel::interrupt_manager *kernel_object::interrupt_manager = nullptr;

    alignas(a9n::kernel::process_manager
    ) char kernel_object::process_manager_buffer[process_manager_size];
    a9n::kernel::process_manager *kernel_object::process_manager = nullptr;

    alignas(a9n::kernel::ipc_manager
    ) char kernel_object::ipc_manager_buffer[ipc_manager_size];
    a9n::kernel::ipc_manager *kernel_object::ipc_manager = nullptr;
}
