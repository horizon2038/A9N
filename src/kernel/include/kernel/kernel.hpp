#ifndef KERNEL_OBJECT
#define KERNEL_OBJECT

#include <kernel/interrupt/interrupt_manager.hpp>
#include <kernel/ipc/ipc_manager.hpp>
#include <kernel/memory/memory_manager.hpp>
#include <kernel/process/process_manager.hpp>

#include <hal/interface/hal.hpp>

namespace a9n::kernel
{

    struct kernel_object
    {
      public:
        alignas(a9n::kernel::memory_manager) static char memory_manager_buffer[];
        static a9n::kernel::memory_manager *memory_manager;

        alignas(a9n::kernel::interrupt_manager
        ) static char interrupt_manager_buffer[];
        static a9n::kernel::interrupt_manager *interrupt_manager;

        alignas(a9n::kernel::process_manager
        ) static char process_manager_buffer[];
        static a9n::kernel::process_manager *process_manager;

        alignas(a9n::kernel::ipc_manager) static char ipc_manager_buffer[];
        static a9n::kernel::ipc_manager *ipc_manager;

      private:
        static constexpr uint64_t memory_manager_size
            = sizeof(a9n::kernel::memory_manager);

        static constexpr uint64_t interrupt_manager_size
            = sizeof(a9n::kernel::interrupt_manager);

        static constexpr uint64_t process_manager_size
            = sizeof(a9n::kernel::process_manager);

        static constexpr uint64_t ipc_manager_size = sizeof(a9n::kernel::ipc_manager
        );
    };
}

#endif
