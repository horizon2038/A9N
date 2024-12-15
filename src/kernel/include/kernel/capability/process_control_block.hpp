#ifndef A9N_KERNEL_CAPABILITY_PROCESS_CONTROL_BLOCK_HPP
#define A9N_KERNEL_CAPABILITY_PROCESS_CONTROL_BLOCK_HPP

#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_types.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/process/process.hpp>

namespace a9n::kernel
{
    class process_control_block final : public capability_component
    {
      public:
        process process_core {};

        process_control_block();

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
}

#endif
