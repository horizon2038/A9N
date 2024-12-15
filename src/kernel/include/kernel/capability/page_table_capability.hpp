#ifndef A9N_KERNEL_PAGE_TABLE_CAPABILITY_HPP
#define A9N_KERNEL_PAGE_TABLE_CAPABILITY_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_local_state.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/memory/memory_type.hpp>

namespace a9n::kernel
{
    inline constexpr capability_slot_data convert_page_table_to_slot_data(const page_table &table)
    {
        capability_slot_data data;

        data[0] = table.address;
        data[1] = table.flags;

        return data;
    }

    inline constexpr page_table convert_slot_data_to_page_table(const capability_slot_data &data)
    {
        page_table table;

        table.address = data[0];
        table.flags   = data[1];

        return table;
    }

    class page_table_capability : public capability_component
    {
      public:
        capability_result execute(process &this_process, capability_slot &this_slot) override
        {
            return capability_error::DEBUG_UNIMPLEMENTED;
        };

        capability_result revoke() override
        {
            return capability_error::DEBUG_UNIMPLEMENTED;
        }

        capability_lookup_result retrieve_slot(a9n::word index) override
        {
            return capability_lookup_error::TERMINAL;
        }

        capability_lookup_result traverse_slot(
            a9n::capability_descriptor descriptor,
            a9n::word                  descriptor_max_bits,
            a9n::word                  descriptor_used_bits
        ) override
        {
            return capability_lookup_error::TERMINAL;
        }

      private:
    };

    // because it is a "processor", the component is a single instance;
    // uniqueness is determined by capability_slot_data.
    inline page_table_capability page_table_capability_core;

    inline kernel_result try_configure_pagetable_slot(capability_slot &slot, const page_table &table)
    {
        if (!table.address)
        {
            return kernel_error::ILLEGAL_ARGUMENT;
        }

        slot.component = &page_table_capability_core;
        slot.type      = capability_type::PAGE_TABLE;
        slot.data      = convert_page_table_to_slot_data(table);

        return {};
    }
}

#endif
