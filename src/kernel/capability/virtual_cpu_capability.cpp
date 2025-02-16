#include <kernel/capability/virtual_cpu_capability.hpp>

namespace a9n::kernel
{
    capability_result virtual_cpu_capability::execute(process &owner, capability_slot &self)
    {
        a9n::word type = 0;
        switch (type)
        {
            case CONFIGURE_ADDRESS_SPACE :
                return configure_address_space(owner, self);

            case READ_STATE :
                return read_state(owner, self);

            case WRITE_STATE :
                return write_state(owner, self);

            case ENTER :
                return enter(owner, self);

            case EXIT :
                return capability_error::DEBUG_UNIMPLEMENTED;

            case INJECT_IRQ :
                return inject_irq(owner, self);

            default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_result
        virtual_cpu_capability::configure_address_space(process &owner, capability_slot &self)
    {
        // configure vcpu to virtual address space capability
        return {};
    }

    capability_result
        virtual_cpu_capability::configure_state_descriptor(process &owner, capability_slot &self)
    {
        return {};
    }

    capability_result virtual_cpu_capability::read_state(process &owner, capability_slot &self)
    {
        return {};
    }

    capability_result virtual_cpu_capability::write_state(process &owner, capability_slot &self)
    {
        return {};
    }

    capability_result virtual_cpu_capability::enter(process &owner, capability_slot &self)
    {
        return {};
    }

    capability_result virtual_cpu_capability::inject_irq(process &owner, capability_slot &self)
    {
        return {};
    }
}
