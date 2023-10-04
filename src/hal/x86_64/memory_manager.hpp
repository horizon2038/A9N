#ifndef X86_64_MEMORY_MANAGER_HPP
#define X86_64_MEMORY_MANAGER_HPP

#include <interface/memory_manager.hpp>

namespace hal::x86_64
{
    class memory_manager final : public hal::interface::memory_manager
    {
        public:
            void init_memory() override;
            void virtual_memory_map() override;
            void virtual_memory_unmap() override;
    };
}

#endif
