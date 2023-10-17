#include "memory_manager.hpp"

#include <library/string.hpp>

namespace hal::x86_64
{
    void init_memory()
    {
        // initialize memory_system;
    }

    void memory_manager::init_virtual_memory(uint64_t top_page_table_address)
    {
        // allocate page_table (pml4) ;
        std::memset(reinterpret_cast<void*>(top_page_table_address), 0, 4096);

        uint64_t kernel_page_table_address = 0x100000;

        std::memcpy(reinterpret_cast<void*>(top_page_table_address), reinterpret_cast<void*>(kernel_page_table_address), 4096);
    }

    void memory_manager::map_virtual_memory
    (
        kernel::process *target_process,
        uint64_t virtual_addresss,
        uint64_t physical_address,
        uint64_t page_count
    )
    {
    }

    void memory_manager::unmap_virtual_memory
    (
        kernel::process *target_process,
        uint64_t virtual_addresss,
        uint64_t page_count
    )
    {
    }
}
