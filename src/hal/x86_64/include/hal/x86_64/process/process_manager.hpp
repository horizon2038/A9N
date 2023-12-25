#ifndef X86_64_CONTEXT_SWITCH_HPP
#define X86_64_CONTEXT_SWITCH_HPP

#include <library/common/types.hpp>
#include <hal/interface/process_manager.hpp>

#include <hal/x86_64/arch/segment_configurator.hpp>
#include <hal/x86_64/memory/paging.hpp>

namespace hal::x86_64
{
    class process_manager final : public hal::interface::process_manager
    {
      public:
        process_manager();
        ~process_manager();
        void switch_context(
            kernel::process *preview_process,
            kernel::process *next_process
        ) override;
        void create_process(
            kernel::process *target_process,
            common::virtual_address entry_point_address
        ) override;
        void delete_process(kernel::process *target_process) override;

      private:
        segment_configurator _segment_configurator;
    };
}

#endif
