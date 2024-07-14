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

    capability_error generic::execute(
        capability_slot *this_slot,
        capability_slot *root_slot,
        ipc_buffer      *buffer
    )
    {
        auto e = decode_operation(*this_slot, *root_slot, *buffer);
        // TODO: handle error
        return {};
    }

    a9n::error generic::decode_operation(
        capability_slot  &this_slot,
        capability_slot  &root_slot,
        const ipc_buffer &buffer
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
                    auto e = convert(this_slot, root_slot, buffer);
                    return e;
                }

            default :
                {
                    a9n::kernel::utility::logger::printk("illegal operaton\n");
                    return -1;
                }
        }

        return 0;
    }

    a9n::error generic::convert(
        capability_slot  &this_slot,
        capability_slot  &root_slot,
        const ipc_buffer &buffer
    )
    {
        using namespace liba9n::capability::convert_argument;

        auto               this_info = create_generic_info(this_slot.data);
        capability_factory factory {};
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
            return -1;
        };

        auto target_root_slot = retrieve_target_root_slot(root_slot, buffer);

        auto count      = buffer.get_message<CAPABILITY_COUNT>();
        auto base_index = buffer.get_message<SLOT_INDEX>();

        for (auto i = 0; i < count; i++)
        {
            if (!this_info.is_allocatable(memory_size_bits, 1))
            {
                return -1;
            }
            this_info.apply_allocate(memory_size_bits);
            auto target_empty_slot
                = target_root_slot->component->retrieve_slot(base_index + i);
            *target_empty_slot = factory.make(
                target_type,
                size_bits,
                this_info.current_watermark()
            );
        }

        this_slot.data = this_info.dump_slot_data();

        return 0;
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
        const capability_slot &root_slot,
        const ipc_buffer      &buffer
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

        auto target_slot = root_slot.component->traverse_slot(
            target_descriptor,
            target_depth,
            0
        );

        return target_slot;
    }

    capability_error generic::revoke()
    {
        return {};
    }

}
