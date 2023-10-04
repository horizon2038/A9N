#ifndef HAL_MEMORY_MANAGER_HPP
#define HAL_MEMORY_MANAGER_HPP

namespace hal::interface
{
    class memory_manager
    {
        public:
            virtual void init_memory() = 0;
            virtual void virtual_memory_map() = 0;
            virtual void virtual_memory_unmap() = 0;
    };
}

#endif
