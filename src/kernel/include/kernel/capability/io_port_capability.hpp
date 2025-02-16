#ifndef A9N_KERNEL_IO_PORT_HPP
#define A9N_KERNEL_IO_PORT_HPP

#include "kernel/capability/capability_result.hpp"
#include <kernel/capability/capability_component.hpp>
#include <kernel/types.hpp>

#include <hal/interface/port_io.hpp>
#include <hal/interface/process_manager.hpp>

namespace a9n::kernel
{
    // HACK
    namespace
    {
        inline constexpr capability_error
            convert_hal_to_capability_error([[maybe_unused]] a9n::hal::hal_error e)
        {
            return capability_error::FATAL;
        }
    }

    struct io_port_address_range
    {
        a9n::word min;
        a9n::word max;
    };

    class io_port_capability : public capability_component
    {
      private:
        enum operation_index : a9n::word
        {
            RESERVED = 0, // descriptor
            OPERATION_TYPE,

            RESERVED_MR_2,
            RESERVED_MR_3,
            RESERVED_MR_4,
            RESERVED_MR_5,

            // for read
            SOURCE_PORT = RESERVED_MR_2,

            // for write
            DESTINATION_PORT = RESERVED_MR_2,
            WRITE_DATA       = RESERVED_MR_3,

            // for mint
            NEW_ADDRESS_MIN        = RESERVED_MR_2,
            NEW_ADDRESS_MAX        = RESERVED_MR_3,
            DESTINATION_NODE       = RESERVED_MR_4,
            DESTINATION_NODE_INDEX = RESERVED_MR_5,
        };

        enum result_index : a9n::word
        {
            IS_SUCCESS,
            ERROR_CODE,

            READ_DATA,
        };

        // "io" Mechanism
        enum operation_type : a9n::word
        {
            OPERATION_NONE,
            OPERATION_READ,
            OPERATION_WRITE,
            OPERATION_MINT,
        };

      public:
        capability_result execute(process &owner, capability_slot &self) override;

        capability_result operation_read(process &owner, capability_slot &self);
        capability_result operation_write(process &owner, capability_slot &self);
        capability_result operation_mint(process &owner, capability_slot &self);

        capability_result revoke(capability_slot &self) override
        {
            // remove all processes from the queue
            return {};
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
    };

    inline constexpr io_port_address_range
        convert_slot_data_to_address_range(capability_slot_data &data)
    {
        return io_port_address_range { .min = data[0], .max = data[1] };
    }

    inline constexpr capability_slot_data
        convert_address_range_to_slot_data(io_port_address_range range)
    {
        capability_slot_data data;
        data[0] = range.min;
        data[1] = range.max;
        return data;
    }

    inline constexpr kernel_result try_configure_io_port_slot(
        capability_slot      &slot,
        io_port_capability   &port,
        io_port_address_range range
    )
    {
        slot.init();
        slot.component = &port;
        slot.type      = capability_type::IO_PORT;
        slot.rights    = capability_slot::ALL;
        slot.data      = convert_address_range_to_slot_data(range);

        return {};
    }
}

#endif
