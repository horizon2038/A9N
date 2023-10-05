#include "paging.hpp"

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
        for (uint16_t i = 0; i <= PAGE_TABLE_COUNT; i++)
        {
            entries[i].all = 0;
        }
    }
    
    page *page_table::page_to_physical_address(uint16_t index)
    {
        return reinterpret_cast<page*>(entries[index].address << 12);
    }
}

