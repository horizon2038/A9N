#ifndef A9N_KERNEL_FRAME_CAPABILITY_HPP
#define A9N_KERNEL_FRAME_CAPABILITY_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_local_state.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/memory/memory_type.hpp>

namespace a9n::kernel
{
    inline constexpr capability_slot_data convert_frame_to_slot_data(const frame &target_frame)
    {
        capability_slot_data data;

        data[0] = target_frame.address;
        data[1] = target_frame.flags;

        return data;
    }

    inline constexpr frame convert_slot_data_to_frame(const capability_slot_data &data)
    {
        frame target_frame;

        target_frame.address = data[0];
        target_frame.flags   = data[1];

        return target_frame;
    }

    class frame_capability : public capability_component
    {
      public:
        capability_result execute(process &this_process, capability_slot &this_slot) override
        {
            return capability_error::DEBUG_UNIMPLEMENTED;
        };

        capability_result revoke(capability_slot &self) override
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
    inline frame_capability frame_capability_core;

    inline kernel_result try_configure_frame_slot(capability_slot &slot, const frame &target_frame)
    {
        if (!target_frame.address)
        {
            return kernel_error::ILLEGAL_ARGUMENT;
        }

        slot.component = &frame_capability_core;
        slot.type      = capability_type::FRAME;
        slot.data      = convert_frame_to_slot_data(target_frame);

        return {};
    }
}

#endif
