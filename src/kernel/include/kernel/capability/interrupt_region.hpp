#ifndef A9N_KERNEL_INTERRUPT_REGION_HPP
#define A9N_KERNEL_INTERRUPT_REGION_HPP

#include <kernel/capability/capability_component.hpp>

namespace a9n::kernel
{
    /*
    class interrupt_region : public capability_component
    {
      private:
        enum operation_index : a9n::word
        {
            RESERVED = 0, // descriptor
            OPERATION_TYPE,

            IRQ_NUMBER,
            TARGET_NODE,
            TARGET_NODE_INDEX,
        };

        enum result_index : a9n::word
        {
            IS_SUCCESS = 0,
            ERROR_CODE,
        };

        enum operation_type : a9n::word
        {
            OPERATION_NONE,
            OPERATION_MAKE_PORT,
        };

      public:
        capability_result execute(process &owner, capability_slot &self) override;

        capability_result operation_make_port(process &owner, capability_slot &self);

        capability_result revoke(capability_slot &self) override;

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
        };
    };
    */
}

#endif
