#include "kernel.hpp"
#include "ipc_manager.hpp"
#include "memory_manager.hpp"
#include "process.hpp"
#include "process_manager.hpp"

namespace kernel
{
    alignas(kernel::memory_manager) char kernel_object::memory_manager_buffer[memory_manager_size];
    kernel::memory_manager *kernel_object::memory_manager = nullptr;

    alignas(kernel::process_manager) char kernel_object::process_manager_buffer[process_manager_size];
    kernel::process_manager *kernel_object::process_manager = nullptr;

    alignas(kernel::ipc_manager) char kernel_object::ipc_manager_buffer[ipc_manager_size];
    kernel::ipc_manager *kernel_object::ipc_manager = nullptr;
}
