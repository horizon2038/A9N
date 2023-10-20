#include "paging.hpp"
#include <library/string.hpp>

namespace hal::x86_64
{
    page_table::page_table()
    {
        init();
    }

    page_table::~page_table()
    {
    }

    void page_table::init()
    {
        /*
        for (uint16_t i = 0; i <= PAGE_TABLE_COUNT; i++)
        {
            entries[i].all = 0;
        }
        */
        std::memset(entries, 0, sizeof(page_table) * PAGE_TABLE_COUNT);
    }
    
    uint64_t page_table::page_to_physical_address(uint16_t index)
    {
        page *entry = &entries[index];
        if (!entry->present)
        {
            return 0;
        }
        return entries[index].address << 12;
    }

    bool page_table::is_present(uint16_t index)
    {
        page *entry = &entries[index];
        return entry->present;
    }
}

