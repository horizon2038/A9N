#include <kernel/capability/frame_capability.hpp>

namespace a9n::kernel
{
    capability_result frame_capability::execute(process &this_process, capability_slot &this_slot)
    {
        return capability_error::DEBUG_UNIMPLEMENTED;
    };

    capability_result frame_capability::operation_get_address(process &owner, capability_slot &self)
    {
        auto target_frame = convert_slot_data_to_frame(self.data);
        auto address      = target_frame.address;
        return a9n::hal::configure_message_register(owner, ADDRESS, address)
            .transform_error(convert_hal_to_kernel_error)
            .transform_error(convert_kernel_to_capability_error);

        return {};
    }
}
