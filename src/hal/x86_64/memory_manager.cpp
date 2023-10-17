#include "memory_manager.hpp"

namespace hal::x86_64
{
    void init_memory()
    {
        // initialize memory_system;
    }

    void memory_manager::init_page_table(uint64_t target_page_table)
    {
        // allocate page_table (pml4) ;
        //
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
