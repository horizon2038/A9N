#ifndef A9N_KERNEL_VIRTUAL_CPU_CAPABILITY_HPP
#define A9N_KERNEL_VIRTUAL_CPU_CAPABILITY_HPP

#include "hal/arch/arch_types.hpp"
#include "kernel/capability/capability_result.hpp"
#include "kernel/types.hpp"
#include <kernel/capability/capability_component.hpp>
#include <kernel/virtualization/virtual_cpu.hpp>

#include <hal/interface/virtualize.hpp>

namespace a9n::kernel
{
    // the API is architecture-independent; but the binary layout is architecture-dependent
    class virtual_cpu_capability : public capability_component
    {
      private:
        enum operation_type : a9n::word
        {
            NONE = 0,
            CONFIGURE_ADDRESS_SPACE, // virtual address space
            CONFIGURE_STATE_DESCRIPTOR,
            READ_STATE,
            WRITE_STATE,
            ENTER,
            EXIT, // reserved ?
            INJECT_IRQ,
        };

        enum operation_index : a9n::word
        {
            INDEX_RESERVED = 0, // descriptor
            INDEX_OPERATION_TYPE,

            // MRs
            INDEX_RESERVED_MR_2,
            INDEX_RESERVED_MR_3,

            // for CONFIGURE_ADDRESS_SPACE
            INDEX_ADDRESS_SPACE = INDEX_RESERVED_MR_2,

            // for CONFIGURE_STATE_DESCRIPTOR
            INDEX_STATE_DESCRIPTOR = INDEX_RESERVED_MR_2,
            INDEX_EXIT_REASON      = INDEX_RESERVED_MR_3,

            // for READ_STATE
            // INDEX_STATE_DESCRIPTOR = INDEX_RESERVED_MR_2, (already defined)

            // for WRITE_STATE
            // INDEX_STATE_DESCRIPTOR = INDEX_RESERVED_MR_2, (already defined)
            INDEX_WRITE_STATE_BASE = INDEX_RESERVED_MR_3,

            // for ENTER
            // INDEX_STATE_DESCRIPTOR = INDEX_RESERVED_MR_2, (already defined)

            // for EXIT
            // unused

            // for INJECT_IRQ
            INDEX_INJECT_IRQ_NUMBER = INDEX_RESERVED_MR_2,
        };

        enum result_index : a9n::word
        {
            // for all operations
            RESULT_IS_SUCCESS,
            RESULT_ERROR_CODE,

            // configure_address_space, configure_state_descriptor, and inject_irq do not have any
            // additional return values.

            // for READ_STATE
            INDEX_READ_STATE_BASE = INDEX_RESERVED_MR_2,

            // for ENTER
            RESULT_STATE_DESCRIPTOR = INDEX_RESERVED_MR_2,
            RESULT_EXIT_REASON      = INDEX_RESERVED_MR_3,
        };

        virtual_cpu core;

        // the virtual cpu descriptor determines the machine state to be transferred upon an exit
        // from virtual machine. this allows for state transfer to be executed only for the
        // necessary parts of the machine state.
        liba9n::std::array<a9n::word, a9n::hal::VIRTUAL_CPU_STATE_COUNT> state_descriptors;

      public:
        capability_result execute(process &owner, capability_slot &self) override;

        capability_result configure_address_space(process &owner, capability_slot &self);
        capability_result configure_state_descriptor(process &owner, capability_slot &self);
        capability_result read_state(process &owner, capability_slot &self);
        capability_result write_state(process &owner, capability_slot &self);
        capability_result enter(process &owner, capability_slot &self);

        // stub
        capability_result exit(process &owner, capability_slot &self)
        {
            return {};
        }

        capability_result inject_irq(process &owner, capability_slot &self);

        capability_result revoke(capability_slot &self) override
        {
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

    inline kernel_result
        try_configure_virtual_cpu_slot(capability_slot &slot, virtual_cpu_capability &vcpu)
    {
        if (slot.type != capability_type::NONE)
        {
            return kernel_error::INIT_FIRST;
        }

        slot.component = &vcpu;
        slot.type      = capability_type::VIRTUAL_CPU;
        slot.rights    = capability_slot::object_rights::ALL;
        slot.data.fill(0);

        return {};
    }

}

#endif
