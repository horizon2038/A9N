#include <kernel/capability/generic.hpp>

#include <kernel/ipc/ipc_buffer.hpp>
#include <kernel/capability/capability_local_state.hpp>
#include <kernel/capability/capability_factory.hpp>

#include <liba9n/capability/generic_operation.hpp>
#include <liba9n/capability/capability_types.hpp>
#include <kernel/types.hpp>
#include <liba9n/common/calculate.hpp>

#include <kernel/utility/logger.hpp>

namespace a9n::kernel
{
    // generic_info
    a9n::physical_address generic_info::current_watermark() const
    {
        return watermark;
    }

    bool generic_info::is_device() const
    {
        return device;
    }

    bool generic_info::is_allocatable(
        a9n::word memory_size_bits,
        a9n::word count
    ) const
    {
        auto end_address = base_address + size();
        auto request_unit_size
            = (static_cast<a9n::word>(1) << memory_size_bits);
        auto aligned_watermark
            = liba9n::align_value(watermark, request_unit_size);
        auto target_end_address
            = aligned_watermark + (request_unit_size * count);

        return (target_end_address < end_address);
    }

    a9n::error generic_info::apply_allocate(a9n::word memory_size_bits)
    {
        auto request_unit_size
            = (static_cast<a9n::word>(1) << memory_size_bits);
        auto aligned_watermark
            = liba9n::align_value(watermark, request_unit_size);
        watermark = aligned_watermark + request_unit_size;

        return 0;
    }

    capability_slot_data generic_info::dump_slot_data() const
    {
        capability_slot_data data;

        data[0] = base_address;
        data[1] = serialize_generic_flags(device, size_bits);
        data[2] = watermark;

        return data;
    }

    // generic
    capability_result generic::execute(
        ipc_buffer      *buffer,
        capability_slot *this_slot,
        capability_slot *root_slot
    )
    {
        return decode_operation(*buffer, *this_slot, *root_slot);
    }

    capability_result generic::decode_operation(
        const ipc_buffer &buffer,
        capability_slot  &this_slot,
        capability_slot  &root_slot
    )
    {
        a9n::kernel::utility::logger::printk("generic : decode_operation\n");
        auto operation_type
            = static_cast<liba9n::capability::generic_operation>(
                buffer.message_tag
            );

        switch (operation_type)
        {
            case liba9n::capability::generic_operation::CONVERT :
                {
                    a9n::kernel::utility::logger::printk("generic : CONVERT\n");
                    return convert(buffer, this_slot, root_slot);
                }

            default :
                {
                    a9n::kernel::utility::logger::printk("illegal operaton\n");
                    return capability_error::ILLEGAL_OPERATION;
                }
        }

        return {};
    }

    capability_result generic::convert(
        const ipc_buffer &buffer,
        capability_slot  &this_slot,
        capability_slot  &root_slot
    )
    {
        using namespace liba9n::capability::convert_argument;

        auto this_info   = create_generic_info(this_slot.data);
        auto target_type = static_cast<liba9n::capability::capability_type>(
            buffer.get_message<CAPABILITY_TYPE>()
        );
        auto size_bits = buffer.get_message<CAPABILITY_SIZE_BITS>();
        auto memory_size_bits
            = factory.calculate_memory_size_bits(target_type, size_bits);

        // "device" Generic can only be converted to a frame object.
        if (this_info.is_device()
            && target_type != liba9n::capability::capability_type::FRAME)
        {
            return capability_error::INVALID_ARGUMENT;
        };

        auto target_root_slot = retrieve_target_root_slot(buffer, root_slot);

        auto count      = buffer.get_message<CAPABILITY_COUNT>();
        auto base_index = buffer.get_message<SLOT_INDEX>();

        for (auto i = 0; i < count; i++)
        {
            if (!this_info.is_allocatable(memory_size_bits, 1))
            {
                return capability_error::INVALID_ARGUMENT;
            }

            this_info.apply_allocate(memory_size_bits);

            // HACK: stop using unwrap()
            auto target_empty_slot
                = target_root_slot->component->retrieve_slot(base_index + i)
                      .unwrap();
            *target_empty_slot = factory.make(
                target_type,
                size_bits,
                this_info.current_watermark()
            );
        }

        // update slot local data
        this_slot.data = this_info.dump_slot_data();

        return {};
    }

    generic_info generic::create_generic_info(const capability_slot_data &data)
    {
        return generic_info {
            data[0],
            generic_flags_size_bits(data[1]),
            generic_flags_is_device(data[1]),
            data[2],
        };
    }

    capability_slot *generic::retrieve_target_root_slot(
        const ipc_buffer      &buffer,
        const capability_slot &root_slot
    ) const
    {
        using namespace liba9n::capability::convert_argument;

        auto target_descriptor = buffer.get_message<ROOT_DESCRIPTOR>();
        auto target_depth      = buffer.get_message<ROOT_DEPTH>();
        auto target_index      = buffer.get_message<SLOT_INDEX>();

        if (target_depth == 0)
        {
            return const_cast<capability_slot *>(&root_slot);
        }

        // HACK: stop using unwrap()
        auto target_slot
            = root_slot.component
                  ->traverse_slot(target_descriptor, target_depth, 0)
                  .unwrap();

        return target_slot;
    }

    capability_result generic::revoke()
    {
        return {};
    }

}
