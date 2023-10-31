#ifndef KERNEL_OBJECT
#define KERNEL_OBJECT

#include "memory_manager.hpp"
#include "interrupt_manager.hpp"
#include "process_manager.hpp"
#include "ipc_manager.hpp"

#include <interface/hal.hpp>

namespace kernel
{
    
    struct kernel_object
    {
        public:
            alignas(kernel::memory_manager) static char memory_manager_buffer[];
            static kernel::memory_manager *memory_manager;

            alignas(kernel::interrupt_manager) static char interrupt_manager_buffer[];
            static kernel::interrupt_manager *interrupt_manager;

            alignas(kernel::process_manager) static char process_manager_buffer[];
            static kernel::process_manager *process_manager;

            alignas(kernel::ipc_manager) static char ipc_manager_buffer[];
            static kernel::ipc_manager *ipc_manager;

        private:
            constexpr static uint64_t memory_manager_size = sizeof(kernel::memory_manager);

            constexpr static uint64_t interrupt_manager_size = sizeof(kernel::interrupt_manager);

            constexpr static uint64_t process_manager_size = sizeof(kernel::process_manager);

            constexpr static uint64_t ipc_manager_size = sizeof(kernel::ipc_manager);
    };
}

#endif
